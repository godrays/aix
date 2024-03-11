//
//  Copyright © 2024-Present, Arkin Terli. All rights reserved.
//
//  NOTICE:  All information contained herein is, and remains the property of Arkin Terli.
//  The intellectual and technical concepts contained herein are proprietary to Arkin Terli
//  and may be covered by U.S. and Foreign Patents, patents in process, and are protected by
//  trade secret or copyright law. Dissemination of this information or reproduction of this
//  material is strictly forbidden unless prior written permission is obtained from Arkin Terli.

// Project includes
#include "Utils.hpp"
#include <aix.hpp>
// External includes
#include <doctest/doctest.h>
// System includes

using namespace aix;

TEST_CASE("Model Forward Test - XOR")
{
    class XORModel : public aix::nn::Module
    {
    public:
        // Constructor
        XORModel(size_t numInputs, size_t numOutputs, size_t numSamples)
        {
            m_w1 = tensor({0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10,
                                0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20}, {numInputs, 10}, true);
            m_b1 = tensor({0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10,
                                0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20,
                                0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10,
                                0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20}, {numSamples, 10}, true);

            m_w2 = tensor({0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10}, {10, 1}, true);
            m_b2 = tensor({0.01, 0.02, 0.03, 0.04}, {numSamples, numOutputs}, true);

            registerParameter(m_w1);
            registerParameter(m_b1);
            registerParameter(m_w2);
            registerParameter(m_b2);
        }

        // Forward
        Tensor forward(Tensor x) const final
        {
            x = Tensor::tanh(Tensor::matmul(x, m_w1) + m_b1);
            x = Tensor::tanh(Tensor::matmul(x, m_w2) + m_b2);
            return x;
        }

        Tensor m_w1, m_b1;
        Tensor m_w2, m_b2;
    };

    // For this test, the following values should not change since the data setup in the test model is static.
    constexpr int kNumSamples  = 4;
    constexpr int kNumInputs   = 2;
    constexpr int kNumTargets  = 1;
    constexpr int kNumHLNodes  = 10;

    // Create a model. Single hidden layer.
    XORModel tm(kNumInputs, kNumTargets, kNumSamples);

    // Example inputs and targets for demonstration purposes.
    auto inputs  = tensor({0.0, 0.0,
                                1.0, 0.0,
                                0.0, 1.0,
                                1.0, 1.0}, {kNumSamples, kNumInputs});

    auto targets = tensor({0.0,
                                1.0,
                                1.0,
                                0.0}, {kNumSamples, kNumTargets});

    auto predictions = tm.forward(inputs);

    predictions.backward();   // ∂m/∂m = [1,1,1,1]  4x1 tensor

    // Check shapes
    CHECK(predictions.value().shape() == std::vector<size_t>{kNumSamples, kNumTargets});
    CHECK(tm.m_w1.grad().shape() == std::vector<size_t>{kNumInputs, kNumHLNodes});
    CHECK(tm.m_b1.grad().shape() == std::vector<size_t>{kNumSamples, kNumHLNodes});
    CHECK(tm.m_w2.grad().shape() == std::vector<size_t>{kNumHLNodes, kNumTargets});
    CHECK(tm.m_b2.grad().shape() == std::vector<size_t>{kNumSamples, kNumTargets});

    // Check values
    CheckVectorApproxValues(predictions.value(), tensor({0.048378, 0.148142, 0.157907, 0.247461}, predictions.value().shape()).value());
    CheckVectorApproxValues(tm.m_w1.grad(),      tensor({0.0185491, 0.0367438, 0.0545242, 0.0718349, 0.0886248, 0.104848, 0.120463, 0.135436, 0.149734, 0.163334, 0.0185196, 0.0366852, 0.0544368, 0.0717191, 0.0884812, 0.104677, 0.120266, 0.135212, 0.149485, 0.163061}, tm.m_w1.grad().shape()).value());
    CheckVectorApproxValues(tm.m_b1.grad(),      tensor({0.0099756, 0.0199452, 0.0299029, 0.0398426, 0.0497585, 0.0596446, 0.0694951, 0.0793041, 0.089066, 0.0987749, 0.00964104, 0.0191826, 0.0286031, 0.0378815, 0.0469976, 0.0559322, 0.0646669, 0.0731846, 0.081469, 0.0895053, 0.00961158, 0.019124, 0.0285157, 0.0377657, 0.046854, 0.0557612, 0.0644693, 0.0729609, 0.0812201, 0.0892318, 0.00890803, 0.0175611, 0.0259211, 0.0339534, 0.0416272, 0.0489158, 0.0557965, 0.0622511, 0.0682653, 0.0738288}, tm.m_b1.grad().shape()).value());
    CheckVectorApproxValues(tm.m_w2.grad(),      tensor({0.455419, 0.530338, 0.604638, 0.678252, 0.751116, 0.82317, 0.894358, 0.964627, 1.03393, 1.10222}, tm.m_w2.grad().shape()).value());
    CheckVectorApproxValues(tm.m_b2.grad(),      tensor({0.99766, 0.978054, 0.975065, 0.938763}, tm.m_b2.grad().shape()).value());
    // Note: Results are consistent with those from PyTorch.
}


TEST_CASE("ones")
{
    SUBCASE("without requiring gradient")
    {
        aix::Tensor t = aix::ones({2, 3});
        CHECK(t.shape() == aix::Shape{2, 3});
        CHECK(t.value().size() == 6);
        CHECK(t.isRequireGrad() == false);

        aix::TensorValue expected(1.0, {2, 3}, t.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }

    SUBCASE("requiring gradient")
    {
        aix::Tensor t = aix::ones({2, 3}, true);
        CHECK(t.shape() == aix::Shape{2, 3});
        CHECK(t.value().size() == 6);
        CHECK(t.isRequireGrad() == true);

        aix::TensorValue expected(1.0, {2, 3}, t.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }
}

TEST_CASE("zeros")
{
    SUBCASE("without requiring gradient")
    {
        aix::Tensor t = aix::zeros({2, 3});
        CHECK(t.shape() == aix::Shape{2, 3});
        CHECK(t.value().size() == 6);
        CHECK(t.isRequireGrad() == false);

        aix::TensorValue expected(0.0, {2, 3}, t.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }

    SUBCASE("requiring gradient")
    {
        aix::Tensor t = aix::zeros({2, 3}, true);
        CHECK(t.shape() == aix::Shape{2, 3});
        CHECK(t.value().size() == 6);
        CHECK(t.isRequireGrad() == true);

        aix::TensorValue expected(0.0, {2, 3}, t.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }
}

TEST_CASE("ones_like")
{
    aix::Tensor src(1.5, {2, 3});

    SUBCASE("without requiring gradient")
    {
        aix::Tensor t = aix::ones_like(src);
        CHECK(t.shape() == src.shape());
        CHECK(t.value().size() == src.value().size());
        CHECK(t.isRequireGrad() == false);
        CHECK(t.value().device() == src.value().device());

        aix::TensorValue expected(1.0, src.shape(), src.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }

    SUBCASE("requiring gradient")
    {
        aix::Tensor t = aix::ones_like(src, true);
        CHECK(t.shape() == src.shape());
        CHECK(t.value().size() == src.value().size());
        CHECK(t.isRequireGrad() == true);
        CHECK(t.value().device() == src.value().device());

        aix::TensorValue expected(1.0, src.shape(), src.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }
}

TEST_CASE("zeros_like")
{
    aix::Tensor src(1.5, {2, 3});

    SUBCASE("without requiring gradient")
    {
        aix::Tensor t = aix::zeros_like(src);
        CHECK(t.shape() == src.shape());
        CHECK(t.value().size() == src.value().size());
        CHECK(t.isRequireGrad() == false);
        CHECK(t.value().device() == src.value().device());

        aix::TensorValue expected(0.0, src.shape(), src.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }

    SUBCASE("requiring gradient")
    {
        aix::Tensor t = aix::zeros_like(src, true);
        CHECK(t.shape() == src.shape());
        CHECK(t.value().size() == src.value().size());
        CHECK(t.isRequireGrad() == true);
        CHECK(t.value().device() == src.value().device());

        aix::TensorValue expected(0.0, src.shape(), src.value().device());
        CheckVectorApproxValues(t.value(), expected);
    }
}
