#pragma once

#include <cmath>
#include <numbers>
#include <limits>
#include <stdexcept>
#include <vector>


class TruncGaussian{
private:
	double mu_;
	double sigma_;
	double a_, b_;
	double Zinv_;
public:
	struct ParamsType{	double mu, sigma, a, b;	};
	static constexpr unsigned n_params = 4;
	
	TruncGaussian(double mu=0, double sigma=1, double a=-std::numeric_limits<double>::infinity(), double b=std::numeric_limits<double>::infinity()) : mu_(mu), sigma_(std::abs(sigma)), a_(a), b_(b), Zinv_(computeZinv(mu_,sigma_,a_,b_)) {};

	TruncGaussian(const double * ptr_params) : TruncGaussian(ptr_params[0], ptr_params[1], ptr_params[2], ptr_params[3]) {};

	TruncGaussian(const ParamsType & params) : TruncGaussian(params.mu, params.sigma, params.a, params.b){};

	void setParams(double mu, double sigma, double a, double b){
		mu_ = mu;
		sigma_ = std::abs(sigma);
		a_ = a;
		b_ = b;
		Zinv_ = computeZinv(mu_, sigma_, a_, b_);
	}

	void setParams(const double * ptr_params){
		this->setParams(ptr_params[0], ptr_params[1], ptr_params[2], ptr_params[3]);
	}

	void setParams(const ParamsType & params){
		this->setParams(params.mu, params.sigma, params.a, params.b);
	}

	ParamsType getParams() const{
		return {mu_, sigma_, a_, b_};
	}
	std::vector<double> getVecParams() const{
		return {mu_, sigma_, a_, b_};
	}

	double operator()(double x) const {
		if (x < a_ || x > b_){
			return 0;
		}
		return Zinv_ * std::exp(-0.5 * std::pow((x - mu_)/sigma_, 2));
	}

	double operator()(double x, double mu, double sigma, double a, double b) const {
		if (x < a || x > b){
			return 0;
		}
		sigma = std::abs(sigma);
		double Zinv = computeZinv(mu, sigma, a, b);
		return Zinv * std::exp(-0.5 * std::pow((x - mu)/sigma, 2));
	}

	double operator()(double x, const double * ptr_params) const {
		return this->operator()(x, ptr_params[0], ptr_params[1], ptr_params[2], ptr_params[3]);
	}

	double operator()(double x, const ParamsType & params) const {
		return this->operator()(x, params.mu, params.sigma, params.a, params.b);
	}

private:
	double computeZinv(double mu, double sigma, double a, double b) const{
		double finite_domain_correction = 0;
		if (b == std::numeric_limits<double>::infinity()){
			finite_domain_correction += Phi((b-mu)/sigma);
		}
		if (a == -std::numeric_limits<double>::infinity()){
			finite_domain_correction -= Phi((a-mu)/sigma);
		}
		return 1 / sigma / std::sqrt(2*std::numbers::pi) / finite_domain_correction;
	}
	double Phi(double x) const{
		return 0.5 * (1 + std::erf(x/std::sqrt(2)));
	}
};