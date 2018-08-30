double parseFloat(const std::string& input)
{
    const char *p = input.c_str();
    if (!*p || *p == '?')
        return NAN_D;
    int s = 1;
    while (*p == ' ') p++;

    if (*p == '-') {
        s = -1; p++;
    }

    double acc = 0;
    while (*p >= '0' && *p <= '9')
        acc = acc * 10 + *p++ - '0';

    if (*p == '.') {
        double k = 0.1;
        p++;
        while (*p >= '0' && *p <= '9') {
            acc += (*p++ - '0') * k;
            k *= 0.1;
        }
    }
    if (*p) die("Invalid numeric format");
    return s * acc;
}
