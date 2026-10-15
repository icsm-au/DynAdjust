// Copyright 2026 Geoscience Australia. Licensed under the Apache License, Version 2.0.
#include "ymsr_report.hpp"

#include <algorithm>
#include <cmath>
#include <include/config/dnaconsts.hpp>
#include <include/functions/dnatemplategeodesyfuncs.hpp>
#include <include/functions/dnatemplatematrixfuncs.hpp>
#include <limits>
#include <stdexcept>

namespace dynadjust {
namespace networkadjust {
namespace {

matrix_2d UnpackCovariance(const std::array<double, 6>& packed) {
    matrix_2d covariance(3, 3);
    size_t element = 0;
    for (UINT32 i = 0; i < 3; ++i)
        for (UINT32 j = i; j < 3; ++j) covariance.put(i, j, packed[element++]);
    covariance.filllower();
    return covariance;
}

std::array<double, 3>
Geographic(const std::array<double, 3>& cartesian, const datum_parameters::CDnaEllipsoid& ellipsoid) {
    std::array<double, 3> geographic{};
    CartToGeo<double>(cartesian[0], cartesian[1], cartesian[2], &geographic[0], &geographic[1], &geographic[2],
                      &ellipsoid);
    // atan2 also handles positions lying exactly on a meridian.
    geographic[1] = std::atan2(cartesian[1], cartesian[0]);
    return geographic;
}

void CalculateStatistics(YReportRow& row, double sigma_zero) {
    // Match the adjustment's treatment of negative residual-variance roundoff.
    row.residual_variance = std::fabs(row.measured_variance - row.adjusted_variance);
    const double residual_sd = std::sqrt(row.residual_variance);
    if (residual_sd == 0.0) {
        row.nstat =
            row.correction == 0.0 ? 0.0 : std::copysign(std::numeric_limits<double>::infinity(), row.correction);
        row.reliability = UNRELIABLE;
    } else {
        row.nstat = row.correction / residual_sd;
        row.reliability = std::sqrt(row.measured_variance) / residual_sd;
        if (!std::isfinite(row.reliability) || row.reliability > STABLE_LIMIT) row.reliability = UNRELIABLE;
    }
    row.tstat = std::fabs(sigma_zero) < PRECISION_1E10 ? 0.0 : row.nstat / sigma_zero;
}

}  // namespace

YReportTriplet BuildYReportTriplet(const YReportInput& input, const datum_parameters::CDnaEllipsoid& ellipsoid) {
    if (input.mode > XYZ_adj_ymsr_ui)
        throw std::runtime_error("Y measurement output units must be in the range 0 to 3.");
    const bool supplied_geographic = input.supplied_type == LLH_type_i || input.supplied_type == LLh_type_i;
    const UINT16 mode =
        input.mode == AS_MEASURED_adj_ymsr_ui ? (supplied_geographic ? PLH_adj_ymsr_ui : XYZ_adj_ymsr_ui) : input.mode;
    YReportTriplet rows{};
    matrix_2d measured_covariance = UnpackCovariance(input.measured_covariance);
    matrix_2d adjusted_covariance = UnpackCovariance(input.adjusted_covariance);
    matrix_2d measured_report(3, 3), adjusted_report(3, 3);
    std::array<double, 3> measured = input.measured, adjusted = input.adjusted, correction = input.correction;
    const bool geographic = mode != XYZ_adj_ymsr_ui;
    const bool orthometric = input.supplied_type == LLH_type_i;
    const char height = orthometric ? 'H' : 'h';
    if (geographic) {
        const auto position = Geographic(input.adjusted, ellipsoid);
        adjusted = position;
        measured = supplied_geographic ? input.supplied_geographic : Geographic(input.measured, ellipsoid);
        if (orthometric) adjusted[2] -= input.geoid_separation;

        if (mode == ENU_adj_ymsr_ui) {
            matrix_2d cartesian_correction(3, 1), local_correction(3, 1);
            for (UINT32 i = 0; i < 3; ++i) cartesian_correction.put(i, 0, input.correction[i]);
            Rotate_CartLocal<double>(cartesian_correction, &local_correction, position[0], position[1]);
            matrix_2d rotation;
            PropagateVariances_CartLocal_Diagonal<double>(measured_covariance, measured_report, position[0],
                                                          position[1], rotation, true);
            PropagateVariances_CartLocal_Diagonal<double>(adjusted_covariance, adjusted_report, position[0],
                                                          position[1], rotation, false);
            // Keep geographic row order: latitude/north, longitude/east, height/up.
            const UINT32 components[3] = {1, 0, 2};
            for (UINT32 i = 0; i < 3; ++i) {
                correction[i] = local_correction.get(components[i], 0);
                rows[i].measured_variance = measured_report.get(components[i], components[i]);
                rows[i].adjusted_variance = adjusted_report.get(components[i], components[i]);
            }
        } else {
            matrix_2d rotation;
            PropagateVariances_GeoCart<double>(measured_covariance, &measured_report, position[0], position[1],
                                               position[2], rotation, &ellipsoid, false, true);
            PropagateVariances_GeoCart<double>(adjusted_covariance, &adjusted_report, position[0], position[1],
                                               position[2], rotation, &ellipsoid, false, false);
            for (UINT32 i = 0; i < 3; ++i) {
                correction[i] = adjusted[i] - measured[i];
                rows[i].measured_variance = measured_report.get(i, i);
                rows[i].adjusted_variance = adjusted_report.get(i, i);
            }
            correction[1] = std::remainder(correction[1], TWO_PI);
        }
    } else {
        for (UINT32 i = 0; i < 3; ++i) {
            rows[i].measured_variance = measured_covariance.get(i, i);
            rows[i].adjusted_variance = adjusted_covariance.get(i, i);
        }
    }
    const char coordinates[3] = {geographic ? 'P' : 'X', geographic ? 'L' : 'Y', geographic ? height : 'Z'};
    const char local_components[3] = {'n', 'e', 'u'};
    for (UINT32 i = 0; i < 3; ++i) {
        auto& row = rows[i];
        row.coordinate = coordinates[i];
        row.label = std::string(1, coordinates[i]);
        if (mode == ENU_adj_ymsr_ui) row.label += local_components[i];
        row.angular_coordinate = geographic && i < 2;
        row.angular_statistics = mode == PLH_adj_ymsr_ui && i < 2;
        row.measured = measured[i];
        row.adjusted = adjusted[i];
        row.correction = correction[i];
        if (geographic && orthometric && i == 2) row.pre_adjustment_correction = input.pre_adjustment_correction;
        CalculateStatistics(row, input.sigma_zero);
    }
    return rows;
}

double YReportSortKey(const YReportTriplet& rows, UINT16 field) {
    double maximum = 0.0;
    for (const auto& row : rows) {
        double value;
        switch (field) {
        case meas_adj_msr_sort_ui: value = row.measured; break;
        case corr_adj_msr_sort_ui: value = row.correction; break;
        case a_sd_adj_msr_sort_ui: value = std::sqrt(row.adjusted_variance); break;
        case n_st_adj_msr_sort_ui: value = row.nstat; break;
        default: return 0.0;
        }
        if (std::isnan(value)) return value;
        maximum = std::max(maximum, std::fabs(value));
    }
    return maximum;
}

bool ReportKeyGreater(double lhs, double rhs) {
    if (std::isnan(lhs)) return false;
    if (std::isnan(rhs)) return true;
    return lhs > rhs;
}

}  // namespace networkadjust
}  // namespace dynadjust
