// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/10/01)

#ifndef WM5MASSSPRINGCURVE_H
#define WM5MASSSPRINGCURVE_H

#include "Wm5PhysicsLIB.h"
#include "Wm5ParticleSystem.h"

namespace Wm5
{

template <typename Real, typename TVector>
class WM5_PHYSICS_ITEM MassSpringCurve : public ParticleSystem<Real,TVector>
{
public:
    // Construction and destruction.  This class represents a set of N-1
    // springs connecting N masses that lie on a curve.  Spring i connects
    // masses i-1 and i for 1 <= i <= N-1.
    MassSpringCurve (int numParticles, Real step);
    virtual ~MassSpringCurve ();

    int GetNumSprings () const;
    Real& Constant (int i);  // spring constant
    Real& Length (int i);  // spring resting length

    // Callback for acceleration (ODE solver uses x" = F/m) applied to
    // particle i.  The positions and velocities are not necessarily
    // m_akPosition and m_akVelocity since the ODE solver evaluates the
    // impulse function at intermediate positions.
    virtual TVector Acceleration (int i, Real time,
        const TVector* positions, const TVector* velocities);

    // The default external force is zero.  Derive a class from this one to
    // provide nonzero external forces such as gravity, wind, friction,
    // and so on.  This function is called by Acceleration(...) to append the
    // acceleration F/m generated by the external force F.
    virtual TVector ExternalAcceleration (int i, Real time,
        const TVector* positions, const TVector* velocities);

protected:
    using ParticleSystem<Real,TVector>::mNumParticles;
    using ParticleSystem<Real,TVector>::mInvMasses;

    int mNumSprings;
    Real* mConstants;
    Real* mLengths;
};

typedef MassSpringCurve<float,Vector2f> MassSpringCurve2f;
typedef MassSpringCurve<double,Vector2d> MassSpringCurve2d;
typedef MassSpringCurve<float,Vector3f> MassSpringCurve3f;
typedef MassSpringCurve<double,Vector3d> MassSpringCurve3d;

}

#endif
