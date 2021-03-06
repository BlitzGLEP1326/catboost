#pragma once

#include "target_base.h"
#include "kernel.h"
#include "target_options.h"
#include <catboost/cuda/targets/kernel/pointwise_targets.cuh>
#include <catboost/cuda/cuda_util/algorithm.h>
#include <catboost/cuda/data/data_provider.h>

namespace NCatboostCuda
{
    template<class TDocLayout, class TDataSet>
    class TCrossEntropy: public TPointwiseTarget<TDocLayout, TDataSet>
    {
    public:
        using TParent = TPointwiseTarget<TDocLayout, TDataSet>;
        using TStat = TAdditiveStatistic;
        using TMapping = TDocLayout;
        CB_DEFINE_CUDA_TARGET_BUFFERS();

        TCrossEntropy(const TDataSet& dataSet,
                      TRandom& random,
                      TSlice slice,
                      const TTargetOptions& targetOptions)
                : TParent(dataSet,
                          random,
                          slice,
                          targetOptions)
        {
        }

        TCrossEntropy(const TCrossEntropy& target,
                      const TSlice& slice)
                : TParent(target,
                          slice)
        {
        }

        TCrossEntropy(const TDataSet& dataSet,
                      TRandom& random,
                      TCudaBuffer<const float, TMapping>&& target,
                      TCudaBuffer<const float, TMapping>&& weights,
                      TCudaBuffer<const ui32, TMapping>&& indices,
                      const TTargetOptions& targetOptions)
                : TParent(dataSet,
                          random,
                          std::move(target),
                          std::move(weights),
                          std::move(indices),
                          targetOptions)
        {
        }

        TCrossEntropy(TCrossEntropy&& other)
                : TParent(std::move(other))
        {
        }

        using TParent::GetWeights;
        using TParent::GetTarget;
        using TParent::GetTargetOptions;
        using TParent::GetTotalWeight;

        TAdditiveStatistic ComputeStats(const TConstVec& point) const
        {
            yvector<float> result;
            auto tmp = TVec::Create(point.GetMapping().RepeatOnAllDevices(1));

            Approximate(GetTarget(),
                        GetWeights(),
                        point,
                        &tmp,
                        nullptr,
                        nullptr);

            NCudaLib::TCudaBufferReader<TVec>(tmp)
                    .SetFactorSlice(TSlice(0, 1))
                    .SetReadSlice(TSlice(0, 1))
                    .ReadReduce(result);
            const double weight = GetTotalWeight();

            return TAdditiveStatistic(result[0], weight);
        }

        static double Score(const TAdditiveStatistic& score)
        {
            return score.Sum / score.Weight;
        }

        double Score(const TConstVec& point)
        {
            return Score(ComputeStats(point));
        }

        void GradientAt(const TConstVec& point,
                        TVec& dst,
                        ui32 stream = 0) const
        {
            Approximate(GetTarget(),
                        GetWeights(),
                        point,
                        nullptr,
                        &dst,
                        nullptr,
                        stream);
        }

        void Approximate(const TConstVec& target,
                         const TConstVec& weights,
                         const TConstVec& point,
                         TVec* value,
                         TVec* der,
                         TVec* der2,
                         ui32 stream = 0) const
        {
            ApproximateCrossEntropy(target,
                                    weights,
                                    point,
                                    value,
                                    der,
                                    der2,
                                    GetTargetOptions().IsUseBorderForClassification(),
                                    GetTargetOptions().GetBinClassBorder(),
                                    stream);
        }

        static constexpr TStringBuf TargetName()
        {
            return "CrossEntropy";
        }

        static constexpr bool IsMinOptimal()
        {
            return false;
        }
    };
}
