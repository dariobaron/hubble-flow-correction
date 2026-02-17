#pragma once

#include <cmath>


template<unsigned DOF>
class Chi{

private:
	double scale_;
	constexpr static double Zinv_ = 1 / std::pow(2, DOF/2.-1) / std::tgamma(DOF/2.);

public:
	struct ParamsType{	double scale;	};
	static constexpr unsigned n_params = 1;

	Chi(double scale=1) : scale_(std::abs(scale)) {};

	Chi(const double * ptr_params) : Chi(ptr_params[0]) {};

	Chi(const ParamsType & params) : Chi(params.scale){};

	void setParams(double scale){
		scale_ = std::abs(scale);
	}

	void setParams(const double * ptr_params){
		this->setParams(ptr_params[0]);
	}

	void setParams(const ParamsType & params){
		this->setParams(params.scale);
	}

	ParamsType getParams() const{
		return {scale_};
	}
	std::vector<double> getVecParams() const{
		return {scale_};
	}

	double operator()(double x) const {
		if (x <= 0){
			return 0;
		}
		double y = x / scale_;
		return Zinv_ / scale_ * std::pow(y, DOF-1) * std::exp(-0.5 * y*y);
	}

	double operator()(double x, double scale) const {
		if (x <= 0){
			return 0;
		}
		double y = x / scale;
		return Zinv_ / scale * std::pow(y, DOF-1) * std::exp(-0.5 * y*y);
	}

	double operator()(double x, const double * ptr_params) const {
		return this->operator()(x, ptr_params[0]);
	}

	double operator()(double x, const ParamsType & params) const {
		return this->operator()(x, params.scale);
	}

};


using TruncGaussian = Chi<1>;
using Rayleigh = Chi<2>;
using MaxwellBoltzmann = Chi<3>;