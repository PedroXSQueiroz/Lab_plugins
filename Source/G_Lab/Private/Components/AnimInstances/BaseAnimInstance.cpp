// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AnimInstances/BaseAnimInstance.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Algo/Reverse.h"

#pragma optimize("", off)
FVector UBaseAnimInstance::GetDesiredDirection()
{
    ACharacter* player = Cast<ACharacter>( this->GetOwningActor() );
    
    if (!player) 
    {
        return FVector::Zero();
    }

    return player->GetLastMovementInputVector();
}

FIKData UBaseAnimInstance::GetIKData(FIKParams& ikParams, bool& hitted)
{
    
    /************
    * GET WEIGHTS
    *************/
    bool getWeightByCurve = ikParams.WeightCurveName.IsValid()
                        &&  !ikParams.WeightCurveName.IsNone()
                        &&  ikParams.WeightCurveName.GetStringLength() > 0;
    float currentWeight = getWeightByCurve ?
            this->GetCurveValue(ikParams.WeightCurveName) 
        :   ikParams.Weight;

    bool getLockWeightByCurve = ikParams.LockWeightCurveName.IsValid() 
                            &&  !ikParams.LockWeightCurveName.IsNone()
                            &&  ikParams.LockWeightCurveName.GetStringLength() > 0;
    float currentLockWeight = getLockWeightByCurve ?
            this->GetCurveValue(ikParams.LockWeightCurveName)
        :   ikParams.LockWeight;


    /**********************
    * CALCULATE START TRACE
    ***********************/
    FVector startTrace = FVector::Zero();
    FVector boneReferenceLocation = FVector::Zero();
    FVector startReference = FVector::Zero();
    if (ikParams.StartTraceBoneReference.IsValid() && ikParams.StartTraceBoneReference.GetStringLength() > 0) 
    {
        boneReferenceLocation = this->GetOwningComponent()->GetSocketLocation(ikParams.StartTraceBoneReference);
        startReference = (boneReferenceLocation * ikParams.StartTraceMask );
        startTrace = FVector(startReference);

        if (ikParams.AddRelativeLocationFromReverseMask) 
        {
            FVector reverseMask = FVector(1) - ikParams.StartTraceMask;
            startTrace += ( reverseMask * ikParams.ReverseMaskStartTraceLocation );
            startReference += (reverseMask * this->GetOwningComponent()->GetComponentLocation());
        }
    }
    else 
    {
        startTrace = ikParams.StartTraceLocation;
    }

    ikParams.ReferenceBoneRotation = this->GetOwningComponent()->GetSocketTransform(ikParams.StartTraceBoneReference, ERelativeTransformSpace::RTS_Component).Rotator();

    /*****************
    * FIND IK LOCATION
    ******************/
    FHitResult traceResult;
    FCollisionQueryParams params;
    
    hitted = this->GetWorld()->SweepSingleByChannel(
        traceResult,
        startTrace,
        startTrace + ( ikParams.TraceDirection * ikParams.TraceLength ),
        FQuat::Identity,
        ECollisionChannel::ECC_Visibility,
        FCollisionShape::MakeSphere(ikParams.TraceRadius),
        params
    );

    if (hitted) 
    {
        FVector rawLockedIKLocation = traceResult.ImpactPoint + ((ikParams.TraceDirection * -1) * ikParams.Padding);
        
        if (this->DebugIKs) 
        {
            DrawDebugSphere(
                this->GetWorld(),
                rawLockedIKLocation,
                12,
                12,
                FColor::Green
            );
        }

        FVector lockedIKLocation = FMath::Lerp( 
                rawLockedIKLocation 
            ,   ikParams.CurrentLockIKLocation
            ,   currentLockWeight
        );

        ikParams.CurrentLockIKLocation = lockedIKLocation;

        FVector additive = (rawLockedIKLocation - startReference - ((ikParams.TraceDirection * -1) * ikParams.Padding)) * (FVector(1) - ikParams.StartTraceMask);

        FVector rawAdditiveIKLocation = boneReferenceLocation + additive;

        FVector additiveIKLocation = FMath::Lerp(
                rawAdditiveIKLocation
            ,   ikParams.CurrentAddtiiveIKLocation
            ,   currentLockWeight
        );

        ikParams.CurrentAddtiiveIKLocation = additiveIKLocation;

        FVector traceDirectionDealocationIK = rawAdditiveIKLocation * (ikParams.TraceDirection * -1);
        FVector perpendicularDealocationIK = FMath::Lerp(
            rawAdditiveIKLocation,
            lockedIKLocation,
            currentLockWeight
        ) * (FVector(1) - (ikParams.TraceDirection * -1) );

        FVector finalIKLocation = traceDirectionDealocationIK + perpendicularDealocationIK;

        if (!ikParams.AlignEffectorBoneToSurface) 
        {
            return FIKData(
                    1
                ,   rawLockedIKLocation
                ,   startReference
                ,   traceResult.Normal
                , finalIKLocation
            );
        }

        float asideAlignment = UKismetMathLibrary::DegAtan2(traceResult.Normal.Y, traceResult.Normal.Z);
        float forwardAlignment = UKismetMathLibrary::DegAtan2(traceResult.Normal.X, traceResult.Normal.Z) * -1;

        FRotator effectorBoneAdditiveRotation = FRotator(
                forwardAlignment + ikParams.EffectorAddtiveRotationOffset.Pitch
            ,   0
            ,   asideAlignment + ikParams.EffectorAddtiveRotationOffset.Roll);

        float rotationWeight = this->GetCurveValue(ikParams.WeightRotationCurveName);
     
        return FIKData(
                1
            ,   rawLockedIKLocation
            ,   startReference
            ,   traceResult.Normal
            , finalIKLocation
            ,   effectorBoneAdditiveRotation
            ,   rotationWeight
        );
    }

    return FIKData();
}
#pragma optimize("", on)

#pragma optimize("", off)
TArray<FIKParams> UBaseAnimInstance::UpdateIKs()
{
    ACharacter* character = Cast<ACharacter>( this->GetOwningActor() );

    if (!character) 
    {
        return TArray<FIKParams>();
    }
    
    TArray<FName> iks;
    this->IKParams.GetKeys(iks);

    for (FName currentIk : iks) 
    {
        bool hitted = false;
        
        FIKData ik = this->GetIKData(this->IKParams[currentIk], hitted);
        this->IKParams[currentIk].StartReferenceLocation = ik.StartReferenceLocation;
        this->IKParams[currentIk].CurrentGlobalIKLocation = ik.Location;
        this->IKParams[currentIk].HitNormal = ik.Normal;
        this->IKParams[currentIk].EffectorAddtiveRotation = ik.Rotation;
        this->IKParams[currentIk].HittedTraceLocation = ik.HittedTraceLocation;
        this->IKParams[currentIk].Hitted = hitted;
        this->IKParams[currentIk].Weight = ik.Weight;
        this->IKParams[currentIk].RotationWeight = ik.RotationWeight;
        this->IKParams[currentIk].FinalIKLocation = this->GetRelativeIKLocation(
            this->IKParams[currentIk].CurrentGlobalIKLocation
        );
    }

    if (this->IsTransitioning) 
    {
        this->InterpolateIKTransition();
    }

    this->UpdateRoots();

    return this->GetIKParamsValues();

}
#pragma optimize("", on)

#pragma optimize("", off)
void UBaseAnimInstance::UpdateRoots()
{
    USkeletalMeshComponent* body = this->GetOwningComponent();

    for (FIKRoots& currentRoot : this->IKRoots)
    {
        currentRoot.RootShouldDealocate = false;

        FVector rootLocation = body->GetSocketLocation(currentRoot.RootReference);

        float excedingDealocation = 0;
        FVector directionDealocation = FVector::Zero();

        for (FName childIK : currentRoot.ChildIKs)
        {
            FVector ikDealocation = this->IKParams[childIK].CurrentGlobalIKLocation - rootLocation;

            float currentExcedingDealocation = (ikDealocation * ( this->IKParams[childIK].TraceDirection * -1 )).Length() - this->IKParams[childIK].MaxLength;

            if (currentExcedingDealocation > 0 && currentExcedingDealocation > excedingDealocation)
            {
                excedingDealocation = currentExcedingDealocation;
                directionDealocation = this->IKParams[childIK].TraceDirection;
                currentRoot.RootShouldDealocate = true;
            }
        }

        if (currentRoot.RootShouldDealocate)
        {
            //TODO: IMPLEMENT OBTAIN OF WEIGHT WITHOUT CURVES

            float rootIKWeight = this->GetCurveValue(currentRoot.RootIKWeightCurveName);
            FVector additionalRootDealocation = (directionDealocation * excedingDealocation * rootIKWeight);

            currentRoot.RootLocation = additionalRootDealocation;
        }

        FVector greaterDealocation = FVector::Zero();
    }
}
#pragma optimize("", on)

void UBaseAnimInstance::UpdateVelocityStats()
{
    FVector currrentVelocity    = this->GetOwningActor()->GetVelocity();
    FVector horizontalVelocity  = FVector(currrentVelocity.X, currrentVelocity.Y, 0);
    float currentAcceleration   = horizontalVelocity.Length() - this->LastVelocity.Length();

    this->IsDecelerating        = currentAcceleration < ( this->IdleMoveThreshold * -1 );
    this->IsAccelerating        = currentAcceleration > this->IdleMoveThreshold;

    if (!this->IsStopping && this->IsDecelerating)
    {
        this->IsStopping = this->GetOwningActor()->GetVelocity().Length() < this->IdleMoveThreshold;
    }

    if (this->IsAccelerating)
    {
        this->IsStopping = false;
    }

    this->LastVelocity = horizontalVelocity;
}

void UBaseAnimInstance::UpdateReverseMaskStartTraceLocation(FName ikName,FVector newLocation)
{
    this->IKParams[ikName].ReverseMaskStartTraceLocation = newLocation;
}

TArray<FIKParams> UBaseAnimInstance::GetIKParamsValues()
{
    TArray<FName> keys;
    TArray<FIKParams> values;
    
    this->IKParams.GetKeys(keys);

    for (FName key : keys) 
    {
        values.Add(this->IKParams[key]);
    }
    
    return values;
}

void UBaseAnimInstance::SetStopping(bool flag)
{
    this->IsStopping = flag;
}

void UBaseAnimInstance::SetInitialIKTransitions(TArray<FTransitIKParams> iksToTransit)
{
    USkeletalMeshComponent* currentBody = this->GetOwningComponent();
    
    if (!currentBody) 
    {
        return;
    }

    for (FTransitIKParams currentIK : iksToTransit)
    {
        if (this->IKParams.Contains(currentIK.IKName)) 
        {
            
            currentIK.InitialRootLocation = currentBody->GetSocketLocation(this->IKParams[currentIK.IKName].RootBone);
            currentIK.InitialRootRotation = currentBody->GetSocketRotation(this->IKParams[currentIK.IKName].RootBone);

            currentIK.InitialLocation = this->IKParams[currentIK.IKName].CurrentGlobalIKLocation;
            
            for (TSubclassOf<UObject> modifierClass : currentIK.Modifier) 
            {
                ITransitionModifier* modifierInstance = Cast<ITransitionModifier>( modifierClass.GetDefaultObject() );
                currentIK.ModifierInstances.Add(modifierInstance);
            }
            
            this->IKTransitionInitialLocation.Add(
                    currentIK.IKName
                ,   currentIK
            );
        }
        else
        {
            //ERROR TREATMENT?
        }
        
    }
}

#pragma optimize("", off)
void UBaseAnimInstance::InterpolateIKTransition()
{
    USkeletalMeshComponent* currentBody = this->GetOwningComponent();
    
    if (!currentBody) 
    {
        return;
    }

    TArray<FName> iks;

    this->IKTransitionInitialLocation.GetKeys(iks);

    FVector rootLocationSum = FVector::Zero();
    FRotator rootRotationSum = FRotator::ZeroRotator;
    
    for (FName ik : iks) 
    {
        
        float rootWeight = this->GetCurveValue(this->IKTransitionInitialLocation[ik].RootCurveName);
        
        FVector currentRootLocation = currentBody->GetSocketLocation(this->IKTransitionInitialLocation[ik].RootBoneReference);
        FRotator currentRootRotation = currentBody->GetSocketRotation(this->IKTransitionInitialLocation[ik].RootBoneReference);

        this->IKTransitionInitialLocation[ik].CurrentRootLocation = 
            FMath::Lerp(
                this->IKTransitionInitialLocation[ik].InitialRootLocation,
                currentRootLocation,
                rootWeight
            );

        this->IKTransitionInitialLocation[ik].CurrrentRootRotation =
            UKismetMathLibrary::RLerp(
                this->IKTransitionInitialLocation[ik].InitialRootRotation,
                currentRootRotation,
                rootWeight,
                true
            );

        rootLocationSum += this->IKTransitionInitialLocation[ik].CurrentRootLocation;
        rootRotationSum = this->IKTransitionInitialLocation[ik].CurrrentRootRotation;

        if (this->DebugIKs) 
        {
            DrawDebugSphere(
                this->GetWorld(),
                this->IKTransitionInitialLocation[ik].CurrentRootLocation,
                24,
                12,
                FColor::Purple
            );
        }


        FTransitIKParams currentTransit = this->IKTransitionInitialLocation[ik];
        FVector currentInitialLocation = currentTransit.InitialLocation;

        float interpWeight = 0;
        if (
            currentTransit.WeightTransitionCurveName.IsValid()
            && !currentTransit.WeightTransitionCurveName.IsNone()
        ) {
            this->GetCurveValue(currentTransit.WeightTransitionCurveName, interpWeight);
        }
        else 
        {
            interpWeight = currentTransit.TargetWeight;
        }

        FVector transitingLocation = FMath::Lerp(currentInitialLocation, this->IKParams[ik].StartReferenceLocation, interpWeight);
        FVector startTrace = FVector(transitingLocation);

        if (this->IKParams[ik].AddRelativeLocationFromReverseMask)
        {
            FVector reverseMask = FVector(1) - this->IKParams[ik].StartTraceMask;

            FVector relativeIncreaseStartTrace = this->GetOwningComponent()
                ->GetComponentTransform()
                .InverseTransformPosition(
                    this->IKParams[ik].ReverseMaskStartTraceLocation
                );

            startTrace += reverseMask * relativeIncreaseStartTrace;
        }

        FHitResult traceResult;
        bool hitted = this->GetWorld()->SweepSingleByChannel(
            traceResult,
            startTrace,
            startTrace + (this->IKParams[ik].TraceDirection * this->IKParams[ik].TraceLength),
            FQuat::Identity,
            ECollisionChannel::ECC_Visibility,
            FCollisionShape::MakeSphere(this->IKParams[ik].TraceRadius)
        );

        
        if (hitted) 
        {
            transitingLocation = traceResult.ImpactPoint +((this->IKParams[ik].TraceDirection * -1) * this->IKParams[ik].Padding);
            this->IKTransitionInitialLocation[ik].TransitLockLocation = transitingLocation;
            this->IKParams[ik].FinalIKLocation = this->GetRelativeIKLocation(transitingLocation);
        }
        else
        {
            this->IKParams[ik].FinalIKLocation = this->GetRelativeIKLocation(transitingLocation);
        }

        for (ITransitionModifier* currentModifier : currentTransit.ModifierInstances) 
        {
            if(currentModifier)
            currentModifier->Execute(this, this->IKParams[ik], currentTransit);
        }

        this->IKParams[ik].CurrentGlobalIKLocation = transitingLocation;
        this->IKParams[ik].CurrentLockIKLocation = transitingLocation;
    }

    this->OverrideRootDuringTranitionLocation = rootLocationSum / this->IKTransitionInitialLocation.Num();
    this->OverrideRootDuringTranitionRotator = rootRotationSum;
    
}
#pragma optimize("", on)

void UBaseAnimInstance::CleanIKTransitions()
{
    this->IKTransitionInitialLocation.Empty();
}

FVector UBaseAnimInstance::GetRelativeIKLocation(FVector ikLocation)
{
    ACharacter* character = Cast<ACharacter>(this->GetOwningActor());

    if (!character)
    {
        return FVector();
    }
    
    return FTransform(
        character->GetMesh()->GetComponentQuat(),
        character->GetMesh()->GetComponentLocation()
    ).InverseTransformPosition(ikLocation);
}

#pragma optimize("", off)
void UTransitionModifierAdditionalHeight::Execute(UBaseAnimInstance* anim, FIKParams& currentParam, FTransitIKParams& transitParams)
{
    float currentHeightRate = 1 - anim->GetCurveValue(this->HeightCurve);

    FVector bodyReferenceLocation = anim->GetOwningComponent()->GetComponentLocation();
    
    FVector relativeLocationAlignedToGround = FTransform(
        anim->GetOwningComponent()->GetComponentQuat(),
        anim->GetOwningComponent()->GetComponentLocation()
    ).InverseTransformPosition(currentParam.HittedTraceLocation);

    currentParam.FinalIKLocation.Z = relativeLocationAlignedToGround.Z + ( currentHeightRate * this->HeightScale );
    
    if (anim->DebugTransitionIKs) 
    {
        DrawDebugSphere(
            anim->GetWorld(),
            transitParams.TransitLockLocation,
            12,
            12,
            FColor::Blue
        );
    }

}
#pragma optimize("", on)

bool UBaseAnimInstance::SetupLean()
{
    
    ACharacter* charac = Cast<ACharacter>(this->GetOwningActor());
    
    for (FLeanParams& lean : this->LeanParans)
    {
        TArray<FLeanBone> chain{ FLeanBone( lean.Effector, FTransform() ) };
        FLeanBone currentBone;

        FName currentBoneName = FName("");

        lean.PreviewLeanAngle = FRotator(
            0,
            charac->GetControlRotation().Yaw,
            0
        );

        do 
        {
            currentBoneName = this->GetOwningComponent()->GetParentBone(chain.Last().Name);

            if (!currentBoneName.IsNone())
            {
                chain.Add(FLeanBone(currentBoneName, FTransform()));
            }

        }while(
                !currentBoneName.IsEqual(lean.Root)
            &&  !currentBoneName.IsNone()
        );

        if (currentBoneName.IsEqual(lean.Root))
        {
            Algo::Reverse(chain);
            lean.BoneChain = chain;
        }
        else 
        {
            return false;
        }

    }

    return false;

}

#pragma optimize("", off)
void UBaseAnimInstance::UpdateLean()
{
    ACharacter* charac = Cast<ACharacter>(this->GetOwningActor());

    if (!charac) 
    {
        return;
    }

    for (int leanIndex = 0; leanIndex < this->LeanParans.Num(); leanIndex++) 
    {
        FLeanParams& lean = this->LeanParans[leanIndex];
        
        if (lean.BoneChain.IsEmpty()) 
        {
            continue;
        }

        if (lean.AxisReference == EAxis::None || lean.AxisEffective == EAxis::None) 
        {
            continue;
        }
        
        float curveOffset = 1.0f / (float) lean.BoneChain.Num();
        float currentCurveStage = 0;
       
        FVector lastInput = charac->GetVelocity();
        lastInput.Normalize();

        if (lastInput.IsZero()) 
        {
            lastInput = charac->GetActorForwardVector();
        }

        FRotator desiredRotationInput = lastInput.Rotation();

        FRotator desiredRotationReference = FRotator(
            lean.AxisReference == EAxis::X ? desiredRotationInput.Pitch: 0,
            lean.AxisReference == EAxis::Z ? desiredRotationInput.Yaw : 0,
            lean.AxisReference == EAxis::Y ? desiredRotationInput.Roll : 0
        );

        FRotator currentRotationInput = charac->GetActorRotation();

        FRotator currentRotationReference = FRotator(
            lean.AxisReference == EAxis::X ? currentRotationInput.Pitch: 0,
            lean.AxisReference == EAxis::Z ? currentRotationInput.Yaw: 0,
            lean.AxisReference == EAxis::Y ? currentRotationInput.Roll: 0
        );


        FRotator rawDiffAngle = UKismetMathLibrary::NormalizedDeltaRotator(currentRotationReference, desiredRotationReference);


        FRotator diffAngle = UKismetMathLibrary::RLerp(
                lean.PreviewDiffLeanAngle
            ,   rawDiffAngle
            ,   lean.Velocity
            ,   false
        );
        
        float referenceAxisValue = 0;
        switch (lean.AxisReference) 
        {
        case EAxis::X: referenceAxisValue = diffAngle.Pitch; break;
        case EAxis::Y: referenceAxisValue = diffAngle.Roll; break;
        case EAxis::Z: referenceAxisValue = diffAngle.Yaw; break;
        }

        FRotator diffApplied = FRotator::ZeroRotator;

        if (lean.Dealocation) 
        {
            float currentIntensity = lean.LeanIntensityCurve.GetRichCurve()->Eval(referenceAxisValue);

            lean.BoneChain[0].Transform.SetLocation(
                FVector(
                    lean.AxisEffective == EAxis::X ? currentIntensity : 0,
                    lean.AxisEffective == EAxis::Z ? currentIntensity : 0,
                    lean.AxisEffective == EAxis::Y? currentIntensity: 0
                )
            );
            lean.BoneChain[0].Dealocation = true;
        }
        else 
        {
            for (FLeanBone& currentBone : lean.BoneChain) 
            {
            
                float currentIntensity = lean.LeanIntensityCurve.GetRichCurve()->Eval(currentCurveStage);

                FRotator currentAdditive = FRotator(
                    lean.MaxAdditiveAngle.Pitch *   (lean.AxisEffective == EAxis::X ? referenceAxisValue : 0) * currentIntensity,
                    lean.MaxAdditiveAngle.Yaw * (lean.AxisEffective == EAxis::Z ? referenceAxisValue : 0) * currentIntensity,
                    lean.MaxAdditiveAngle.Roll * (lean.AxisEffective == EAxis::Y ? referenceAxisValue : 0)   * currentIntensity
                ) - diffApplied;

                currentBone.Transform.SetRotation(
                    currentAdditive.Quaternion()
                );
            
                currentCurveStage += curveOffset;
                diffApplied += currentAdditive;

            }
        }

        lean.PreviewDiffLeanAngle = diffAngle;
        lean.PreviewLeanAngle = desiredRotationReference;
    }
}
#pragma optimize("", on)

TArray<FLean> UBaseAnimInstance::GetLeans()
{
    TArray leans = TArray<FLean>();
    
    for (FLeanParams leanParam : this->LeanParans) 
    {
        leans.Add(FLean(leanParam.BoneChain, leanParam.Dealocation));
    }

    return leans;
}