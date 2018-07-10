#pragma once
#include <glm\vec3.hpp>

struct PhaseTerm
{
	int ePhaseFunc;					//!< Phase function this term uses
	glm::vec3 vDensity;			    //!< Optical density in [R,G,B]
	float fEccentricity;		    //!< Degree/direction of anisotropy (-1, 1) (HG only)
};

struct MediumDescription
{
	glm::vec3 vAbsorption;		//!< Absorpsive component of the medium
	uint32_t uNumPhaseTerms;    //!< Number of valid phase terms
	PhaseTerm phaseTerms[4];
};