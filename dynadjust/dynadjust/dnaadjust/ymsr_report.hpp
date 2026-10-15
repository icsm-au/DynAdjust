// Copyright 2026 Geoscience Australia. Licensed under the Apache License, Version 2.0.
#ifndef DNAADJUST_YMSR_REPORT_HPP_
#define DNAADJUST_YMSR_REPORT_HPP_

#include <array>
#include <include/config/dnaexports.hpp>
#include <include/config/dnatypes-gui.hpp>
#include <include/config/dnatypes.hpp>
#include <include/parameters/dnaellipsoid.hpp>
#include <string>

namespace dynadjust {
namespace networkadjust {

// Covariances use the adjustment's packed upper-triangle order: XX, XY, XZ, YY, YZ, ZZ.
struct YReportInput {
    UINT16 mode = 0;
    _COORD_TYPE_ supplied_type = XYZ_type_i;
    std::array<double, 3> measured{}, adjusted{}, correction{}, supplied_geographic{};
    std::array<double, 6> measured_covariance{}, adjusted_covariance{};
    double geoid_separation = 0.0;
    double pre_adjustment_correction = 0.0;
    double sigma_zero = 1.0;
};

struct YReportRow {
    std::string label;
    char coordinate = 'X';
    bool angular_coordinate = false;
    bool angular_statistics = false;
    double measured = 0.0, adjusted = 0.0, correction = 0.0;
    double measured_variance = 0.0, adjusted_variance = 0.0, residual_variance = 0.0;
    double nstat = 0.0, tstat = 0.0, reliability = 0.0, pre_adjustment_correction = 0.0;
};

using YReportTriplet = std::array<YReportRow, 3>;

DNAADJUST_API YReportTriplet BuildYReportTriplet(const YReportInput& input,
                                                 const dynadjust::datum_parameters::CDnaEllipsoid& ellipsoid);
DNAADJUST_API double YReportSortKey(const YReportTriplet& rows, UINT16 field);
DNAADJUST_API bool ReportKeyGreater(double lhs, double rhs);

}  // namespace networkadjust
}  // namespace dynadjust
#endif
