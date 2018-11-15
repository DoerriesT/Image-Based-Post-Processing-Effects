#pragma once

namespace App
{
	struct Timings
	{
		double m_gtaoRenderTime;
		double m_gtaoSpatialDenoiseTime;
		double m_gtaoTemporalDenoiseTime;
		double m_hbaoRenderTime;
		double m_bilateralBlurRenderTime;
		double m_originalSsaoRenderTime;
		double m_ssaoRenderTime;
		double m_cocComputeTime;
		double m_seperateDofBlurComputeTime;
		double m_seperateDofCompositeComputeTime;
		double m_seperateDofFillComputeTime;
		double m_simpleDofCocBlurComputeTime;
		double m_simpleDofCompositeComputeTime;
		double m_spriteDofCompositeComputeTime;
		double m_cocNeighborTileMaxRenderTime;
		double m_cocTileMaxRenderTime;
		double m_spriteDofRenderTime;
		double m_simpleDofBlurComputeTime;
		double m_seperateDofDownsampleComputeTime;
		double m_velocityCorrectionComputeTime;
		double m_velocityTileMaxRenderTime;
		double m_velocityNeighborTileMaxRenderTime;
		double m_motionBlurRenderTime;
		double m_gtaoSum;
		double m_hbaoSum;
		double m_ssaoSum;
		double m_ssaoOriginalSum;
		double m_simpleDofSum;
		double m_spriteDofSum;
		double m_tiledDofSum;
		double m_simpleMbSum;
		double m_singleDirectionMbSum;
		double m_multiDirectionMbSum;

		Timings &operator+=(const Timings &rhs);
		Timings &operator*=(double rhs);
	};
}