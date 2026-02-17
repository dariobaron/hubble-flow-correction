#pragma once

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>


class Gaussian2D{
	
private:
	double mu_x_, mu_y_;
	double sigma_x_, sigma_y_;
	double rho_;
	double Zinv_;

public:
	struct ParamsType{	double mu_x, mu_y, sigma_x, sigma_y, rho;	};
	static constexpr unsigned n_params = 5;
	
	Gaussian2D(double mu_x=0, double mu_y=0, double sigma_x=1, double sigma_y=1, double rho=0) : mu_x_(mu_x), mu_y_(mu_y), sigma_x_(sigma_x), sigma_y_(sigma_y), rho_(rho), Zinv_(computeZinv(sigma_x,sigma_y,rho)) {
		checkSigmaPositive();
		checkRhoValid();
	};
	
	Gaussian2D(const double * ptr_params) : Gaussian2D(ptr_params[0], ptr_params[1], ptr_params[2], ptr_params[3], ptr_params[4]) {};
	
	Gaussian2D(const ParamsType & params) : Gaussian2D(params.mu_x, params.mu_y, params.sigma_x, params.sigma_y, params.rho) {};

	void setParams(double mu_x, double mu_y, double sigma_x, double sigma_y, double rho){
		mu_x_ = mu_x;
		mu_y_ = mu_y;
		sigma_x_ = sigma_x;
		sigma_y_ = sigma_y;
		rho_ = rho;
		checkSigmaPositive();
		checkRhoValid();
		Zinv_ = computeZinv(sigma_x_, sigma_y_, rho_);
	}

	void setParams(const double * ptr_params){
		this->setParams(ptr_params[0], ptr_params[1], ptr_params[2], ptr_params[3], ptr_params[4]);
	}

	void setParams(const ParamsType & params){
		this->setParams(params.mu_x, params.mu_y, params.sigma_x, params.sigma_y, params.rho);
	}

	ParamsType getParams() const{
		return {mu_x_, mu_y_, sigma_x_, sigma_y_, rho_};
	}
	std::vector<double> getVecParams() const{
		return {mu_x_, mu_y_, sigma_x_, sigma_y_, rho_};
	}

	double operator()(double x, double y) const {
		return Zinv_ * std::exp(-0.5 / (1 - rho_*rho_) * (std::pow((x-mu_x_)/sigma_x_, 2) + std::pow((y-mu_y_)/sigma_y_, 2) + 2*rho_*(x-mu_x_)*(y-mu_y_)/sigma_x_/sigma_y_));
	}

	double operator()(double x, double y, double mu_x, double mu_y, double sigma_x, double sigma_y, double rho) const {
		checkSigmaPositive(sigma_x, sigma_y);
		checkRhoValid(rho);
		double Zinv = computeZinv(sigma_x, sigma_y, rho);
		return Zinv * std::exp(-0.5 / (1 - rho*rho) * (std::pow((x-mu_x)/sigma_x, 2) + std::pow((y-mu_y)/sigma_y, 2) + 2*rho*(x-mu_x)*(y-mu_y)/sigma_x/sigma_y));
	}

	double operator()(double x, double y, const double * ptr_params) const {
		return this->operator()(x, y, ptr_params[0], ptr_params[1], ptr_params[2], ptr_params[3], ptr_params[4]);
	}

	double operator()(double x, double y, const ParamsType & params) const {
		return this->operator()(x, y, params.mu_x, params.mu_y, params.sigma_x, params.sigma_y, params.rho);
	}

private:
	double computeZinv(double sigma_x, double sigma_y, double rho) const{
		return 1. / 2 / std::numbers::pi / sigma_x / sigma_y / std::sqrt(1-rho*rho);
	}

	void checkSigmaPositive() const{
		if (sigma_x_ <= 0 || sigma_y_ <= 0){
			throw std::runtime_error("The standard deviations must be positive");
		}
	}
	
	void checkSigmaPositive(double sigma_x, double sigma_y) const{
		if (sigma_x <= 0 || sigma_y <= 0){
			throw std::runtime_error("The standard deviation must be positive");
		}
	}

	void checkRhoValid() const{
		if (rho_ < -1 || rho_ > 1){
			throw std::runtime_error("The correlation coefficient must be in the range [-1, 1]");
		}
	}

	void checkRhoValid(double rho) const{
		if (rho < -1 || rho > 1){
			throw std::runtime_error("The correlation coefficient must be in the range [-1, 1]");
		}
	}
};