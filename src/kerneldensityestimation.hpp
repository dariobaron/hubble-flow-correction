#pragma once

#include <vector>
#include <numeric>
#include <limits>
#include <iostream>
#include <algorithm>
#include "mixlightlib/integrator.hpp"
#include "../hyperparameters.hpp"


constexpr double infinity = std::numeric_limits<double>::infinity();

template<typename Kernel>
class KernelDensityEstimation{

private:
	Kernel kernel_;
	std::vector<double> values_;
	std::vector<double> weights_;
	double bandwidth_;
	double a_, b_;
	double Zinv_;
	bool binned_;

public:
	// constructors
	KernelDensityEstimation(const double* ptr, size_t N, double bandwidth=1, double a=-infinity, double b=infinity) : kernel_(), values_(ptr, ptr+N), bandwidth_(bandwidth), a_(a), b_(b), Zinv_(), binned_(false) {
		std::sort(values_.begin(), values_.end());
		computeZinv(a_, b_);
	};

	KernelDensityEstimation(const std::vector<double>& values, double bandwidth=1, double a=-infinity, double b=infinity) : KernelDensityEstimation(values.data(), values.size(), bandwidth, a, b) {};

	KernelDensityEstimation(Kernel f, const std::vector<double>& values, double bandwidth=1, double a=-infinity, double b=infinity) : kernel_(f), values_(values), bandwidth_(bandwidth), a_(a), b_(b), Zinv_(), binned_(false) {
		std::sort(values_.begin(), values_.end());
		computeZinv(a_, b_);
	};

	KernelDensityEstimation(const double* ptr_values, size_t N_values, const double* ptr_bins, size_t N_bins, double bandwidth=1, double a=-infinity, double b=infinity) : KernelDensityEstimation(std::vector<double>(ptr_values, ptr_values+N_values), std::vector<double>(ptr_bins, ptr_bins+N_bins), bandwidth, a, b) {};

	KernelDensityEstimation(std::vector<double> values, std::vector<double> bins, double bandwidth=1, double a=-infinity, double b=infinity) : kernel_(), values_(values), bandwidth_(bandwidth), a_(a), b_(b), Zinv_(), binned_(true) {
		binValues(bins);
	};

	// call operator
	double operator()(double x) const {
		return Zinv_ * evalUnnormalized(x);
	};

	// bin values
	void binValues(std::vector<double> bin_edges) {
		std::sort(bin_edges.begin(), bin_edges.end());
		std::vector<unsigned long> occurrences(bin_edges.size()-1, 0);
		for (auto v : values_) {
			if (v < bin_edges.front() || v > bin_edges.back()) {
				continue;
			}
			if (v == bin_edges.front()) {
				++occurrences.front();
			}
			else if (v == bin_edges.back()) {
				++occurrences.back();
			}
			else if (v > bin_edges.front() && v < bin_edges.back()) {
				auto it = std::lower_bound(bin_edges.begin(), bin_edges.end(), v);
				unsigned idx_bin = std::distance(bin_edges.begin(), it) - 1;
				++occurrences[idx_bin];
			}
		}
		weights_.clear();
		weights_.reserve(occurrences.size());
		double tot_occurences = std::accumulate(occurrences.begin(), occurrences.end(), 0.0);
		for (auto count : occurrences) {
			weights_.push_back(count / tot_occurences);
		}
		values_.resize(bin_edges.size()-1);
		for (unsigned i = 0; i < values_.size(); ++i) {
			values_[i] = 0.5 * (bin_edges[i] + bin_edges[i+1]);
		}
		binned_ = true;
		computeZinv(a_, b_);
	}
	
	double evalUnnormalized(double x) const{
		if (x < a_ || x > b_) {
			return 0;
		}
		double result;
		if (binned_) {
			auto operate = [this, x](double v, double w) {
				return w * kernel_((x - v) / bandwidth_);
			};
			result = std::inner_product(values_.begin(), values_.end(), weights_.begin(), 0.0, std::plus<double>(), operate);
		}
		else {
			auto operate = [this, x](double acc, double v){
				return acc + 1. / values_.size() * kernel_((x - v) / bandwidth_);
			};
			result = std::accumulate(values_.begin(), values_.end(), 0., operate);
		}
		return result / bandwidth_;
	};

private:
	void computeZinv(double a, double b){
		double max_element = std::min(b, *std::max_element(values_.begin(), values_.end()));
		double min_element = std::max(a, *std::min_element(values_.begin(), values_.end()));
		double scale = 0.5 * (max_element - min_element);
		double loc = 0.5 * (max_element + min_element);
		Integrator integrator([this](double x){ return evalUnnormalized(x); }, a, b, tolerance_integrals, scale, loc);
		if (!integrator.success()){
			std::cerr << "The normalization of the KernelDensityEstimation at " << this << " failed" << std::endl;
		}
		if (integrator.result() == 0){
			std::cerr << "The normalization of the KernelDensityEstimation at " << this << " is zero" << std::endl;
		}
		Zinv_ = 1 / integrator.result();
	};
};
