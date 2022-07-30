
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_RNG_H
#define COMMON_RNG_H

#include <memory>
#include <random>
#include <memory>
#include <chrono>

enum class DistributionType {
    kNone=0, kNormal=1, kGamma=2, kExponential=3, kUniform=4
};

class Distribution {
protected:
    std::default_random_engine generator_;

public:
    void Seed() { generator_ = std::default_random_engine(std::chrono::steady_clock::now().time_since_epoch().count()); }
    void Seed(size_t seed) { generator_ = std::default_random_engine(seed); }

    virtual void Shape(size_t p1) { throw 1; }
    virtual void Shape(double p1) { throw 1; }
    virtual void Shape(double p1, double p2) { throw 1; }
    virtual int GetInt() { throw 1; }
    virtual size_t GetSize() { throw 1; }
    virtual double GetDouble() { throw 1; }
};

typedef std::unique_ptr<Distribution> DistributionPtr;

class CountDistribution : public Distribution {
private:
    size_t inc_ = 1;
    size_t count_ = 0;
public:
    virtual void Shape(size_t inc) { inc_ = inc; }
    virtual int GetInt() { int temp = count_; count_+=inc_; return temp; }
    virtual size_t GetSize() { size_t temp = count_; count_+=inc_; return temp; };
    virtual double GetDouble() { double temp = count_; count_+=inc_; return temp; };
};

class NormalDistribution : public Distribution {
private:
    std::normal_distribution<double> distribution_;
    //TODO: add binomial dist
public:
    NormalDistribution() = default;
    void Shape(double std) override { distribution_ = std::normal_distribution<double>(0, std); }
    void Shape(double mean, double std) override { distribution_ = std::normal_distribution<double>(mean, std); }
    int GetInt() override { return (int)round(distribution_(generator_)); }
    size_t GetSize() override { return (size_t)round(distribution_(generator_)); }
    double GetDouble() override { return round(distribution_(generator_)); }
};

class GammaDistribution : public Distribution {
private:
    std::gamma_distribution<double> distribution_;
    //TODO: Is there a discrete gamma dist?
public:
    GammaDistribution() = default;
    void Shape(double scale) override { distribution_ = std::gamma_distribution<double>(1, scale); }
    void Shape(double shape, double scale) override { distribution_ = std::gamma_distribution<double>(shape, scale); }
    int GetInt() override { return (int)round(distribution_(generator_)); }
    size_t GetSize() override { return (size_t)round(distribution_(generator_)); }
    double GetDouble() override { return round(distribution_(generator_)); }
};

class ExponentialDistribution : public Distribution {
private:
    std::exponential_distribution<double> distribution_;
    //TODO: add poisson dist
public:
    ExponentialDistribution() = default;
    void Shape(double scale) override { distribution_ = std::exponential_distribution<double>(scale); }
    int GetInt() override { return (int)round(distribution_(generator_)); }
    size_t GetSize() override { return (size_t)round(distribution_(generator_)); }
    double GetDouble() override { return round(distribution_(generator_)); }
};

class UniformDistribution : public Distribution {
private:
    std::uniform_real_distribution<double> distribution_;
    //TODO: add int uniform dist
public:
    UniformDistribution() = default;
    void Shape(size_t high) override { distribution_ = std::uniform_real_distribution<double>(0, (double)high); }
    void Shape(double high) override { distribution_ = std::uniform_real_distribution<double>(0, high); }
    void Shape(double low, double high) override { distribution_ = std::uniform_real_distribution<double>(low, high); }
    int GetInt() override { return (int)round(distribution_(generator_)); }
    size_t GetSize() override { return (size_t)round(distribution_(generator_)); }
    double GetDouble() override { return round(distribution_(generator_)); }
};

class DistributionFactory {
public:
    static DistributionPtr Get(DistributionType type) {
        switch(type) {
            case DistributionType::kNone: {
                return std::make_unique<CountDistribution>();
            }
            case DistributionType::kNormal: {
                return std::make_unique<NormalDistribution>();
            }
            case DistributionType::kGamma: {
                return std::make_unique<GammaDistribution>();
            }
            case DistributionType::kExponential: {
                return std::make_unique<ExponentialDistribution>();
            }
            case DistributionType::kUniform: {
                return std::make_unique<UniformDistribution>();
            }
        }
    }
};

#endif //COMMON_RNG_H