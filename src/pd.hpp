class PD
{
private:
    double target, p, d, last;

public:
    PD (double target, double p, double d)
    : target(target), last(target), p(p), d(d)
    {}

    double update (double measurement)
    {
        const double error = measurement - target;
        const double correction = p * error + d * (error - last);
        last = error;
        return correction;
    }
};