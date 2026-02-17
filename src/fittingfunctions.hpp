#pragma once

#include <cmath>
#include <stdexcept>
#include "kerneldensityestimation.hpp"


double velocityCorrection(double R, double a, double b, double c, double m, double s, double d, double h){
	double hubble_flow = h * R;
	double infall_term = (a + m * std::pow(R, c)) / (std::pow(R, c) - b);
	double transition_term = 1. / (1. + std::exp(-s * (R-d)));
	return (hubble_flow + infall_term) * transition_term;
}


double logSigmaFittingCurve(double M, double logM0, double logA){
	return logA - M / std::pow(10, logM0);
}


double scaleFittingCurve(double R, double M, double f_0, double a, double logM0, double logA){
	double f_inf = std::exp(logSigmaFittingCurve(M, logM0, logA));
	return f_inf + (f_0 - f_inf) * std::exp(-a*R);
}


double invdegFittingCurve(double R, double a, double b, double c, double d, double m, double s){
	double transition = 1 / (1 + std::exp(-s * (R-m)));
	double curve = (a + b * std::pow(R, c)) / (std::pow(R, c) + d);
	return curve * transition;
}


double skewFittingCurve(double R, double a, double b, double c){
	return a * std::pow(R, b) * std::exp(-c*R);
}


class PVcorrclass{
private:
	double a_;
	double b_;
	double c_;
	double m_;
	double s_;
	double d_;
	double h_;
public:
	PVcorrclass() = default;
	PVcorrclass(double a, double b, double c, double m, double s, double d, double h)
		: a_(a), b_(b), c_(c), m_(m), s_(s), d_(d), h_(h) {};
	double operator()(double R) const {
		return velocityCorrection(R, a_, b_, c_, m_, s_, d_, h_);
	};
};


class PUclass{
private:
	double f_0;
	double a_scale;
	double logM0_scale;
	double logA_scale;
	double a_invdeg;
	double b_invdeg;
	double c_invdeg;
	double d_invdeg;
	double m_invdeg;
	double s_invdeg;
public:
	PUclass() = default;
	PUclass(double f_0, double a_scale, double logM0_scale, double logA_scale, 
	   double a_invdeg, double b_invdeg, double c_invdeg, double d_invdeg, double m_invdeg, double s_invdeg)
	   : f_0(f_0), a_scale(a_scale), logM0_scale(logM0_scale), logA_scale(logA_scale),
	     a_invdeg(a_invdeg), b_invdeg(b_invdeg), c_invdeg(c_invdeg), d_invdeg(d_invdeg), m_invdeg(m_invdeg), s_invdeg(s_invdeg) {};
	double operator()(double u, double R, double M) const {
		double scale = scaleFittingCurve(R, M, f_0, a_scale, logM0_scale, logA_scale);
		double invdeg = invdegFittingCurve(R, a_invdeg, b_invdeg, c_invdeg, d_invdeg, m_invdeg, s_invdeg);
		double deg = 1. / invdeg;
		double Z = 1 / std::sqrt(deg) / std::beta(0.5, deg / 2);
		double base = 1 + (u / scale) * (u / scale) / deg;
		double exponent = - (deg + 1) / 2;
		return 1 / Z * std::pow(base, exponent);
	}
};


class unnormalizedPosteriorClass{
private:
	KernelDensityEstimation<KernelTypeKDE> PR_;
	PVcorrclass PVcorr_;
	PUclass PU_;
	double H0_;
public:
	unnormalizedPosteriorClass(const KernelDensityEstimation<KernelTypeKDE> & PR, const PVcorrclass & PVcorr, const PUclass & PU, double H0)
		: PR_(PR), PVcorr_(PVcorr), PU_(PU), H0_(H0) {};
	double geometricComponent(double R, double r) const {
		if (R <= r){
			return 0;
		}
		return r / R / std::sqrt(R*R - r*r);
	};
	double velocityComponent(double R, double r, double v, double M) const {
		if (R <= r){
			return 0;
		}
		double pv = PVcorr_(R);
		double u_plus = v + std::sqrt(1 - r*r/R/R) * pv;
		double u_minus = v - std::sqrt(1 - r*r/R/R) * pv;
		double pu_plus = PU_(u_plus, R, M);
		double pu_minus = PU_(u_minus, R, M);
		return 0.5 * (pu_plus + pu_minus);
	};
	double operator()(double R, double r, double v, double M) const {
		if (R <= r){
			return 0;
		}
		return PR_(R) * geometricComponent(R, r) * velocityComponent(R, r, v, M);
	};
};

