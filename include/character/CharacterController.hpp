#pragma once
#include <Jolt/Physics/Character/Character.h>

class CharacterController
{
    JPH_IMPLEMENT_RTTI_VIRTUAL(CharacterTest)
    {
        JPH_ADD_BASE_CLASS(CharacterTest, CharacterBaseTest)
    }

    static const float cCollisionTolerance = 0.05f;

    CharacterTest::~CharacterTest()
    {
        mCharacter->RemoveFromPhysicsSystem();
    }

    void CharacterTest::Initialize()
    {
        CharacterBaseTest::Initialize();

        // Create 'player' character
        Ref<CharacterSettings> settings = new CharacterSettings();
        settings->mMaxSlopeAngle = DegreesToRadians(45.0f);
        settings->mLayer = Layers::MOVING;
        settings->mShape = mStandingShape;
        settings->mFriction = 0.5f;
        settings->mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
        mCharacter = new Character(settings, RVec3::sZero(), Quat::sIdentity(), 0, mPhysicsSystem);
        mCharacter->AddToPhysicsSystem(EActivation::Activate);
    }
};
