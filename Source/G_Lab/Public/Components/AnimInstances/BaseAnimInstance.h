// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

/****
* IKS
*****/
USTRUCT(BlueprintType)
struct FIKParams 
{
	GENERATED_BODY()

public:
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Weight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName WeightCurveName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float LockWeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName LockWeightCurveName;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName WeightRotationCurveName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RotationWeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D RotationFactor;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootBone;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName EffectorBone;

	UPROPERTY()
	FVector StartReferenceLocation;

	//FIXME: SHOULD BE A QUATERNION
	UPROPERTY()
	FRotator ReferenceBoneRotation;

	UPROPERTY()
	FVector StartTraceLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName StartTraceBoneReference;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool AddRelativeLocationFromReverseMask;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool AlignEffectorBoneToSurface;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector StartTraceMask{ FVector::Zero() };

	UPROPERTY(BlueprintReadWrite)
	FVector ReverseMaskStartTraceLocation{ FVector::Zero() };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector TraceDirection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TraceLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Padding;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TraceRadius;

	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal;

	UPROPERTY(BlueprintReadWrite)
	FVector HittedTraceLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentGlobalIKLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentLockIKLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentAddtiiveIKLocation;

	UPROPERTY(BlueprintReadOnly)
	FRotator EffectorAddtiveRotation;

	UPROPERTY(BlueprintReadWrite)
	FRotator EffectorAddtiveRotationOffset;

	UPROPERTY(BlueprintReadWrite)
	FVector FinalIKLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator FinalIKRotation;

	UPROPERTY(BlueprintReadOnly)
	bool Hitted;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxLength;

};

USTRUCT(BlueprintType)
struct FIKData
{
	GENERATED_BODY()

public:

	FIKData() {};

	FIKData(
			float weight
		,	FVector hittedTraceLocation
		,	FVector reference
		,	FVector normal
		,	FVector location
		,	FRotator rotation = FRotator::ZeroRotator
		,	float rotationWeight = 0
	): 
		StartReferenceLocation(reference)
	,	HittedTraceLocation(hittedTraceLocation)
	,	Location(location)
	,	Weight(weight)
	,	Rotation(rotation)
	,	RotationWeight(rotationWeight)
	{};

	UPROPERTY()
	FVector StartReferenceLocation;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FVector HittedTraceLocation;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector Normal;

	UPROPERTY()
	float Weight;

	UPROPERTY()
	float RotationWeight;

};

USTRUCT(BlueprintType, Blueprintable)
struct FIKRoots
{

	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootReference;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FName> ChildIKs;

	UPROPERTY(BlueprintReadOnly)
	bool RootShouldDealocate;

	UPROPERTY(BlueprintReadOnly)
	FVector RootLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootIKWeightCurveName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RootIKWeight;

};


/************
* TRANSITIONS
*************/
UINTERFACE()
class UTransitionModifier : public UInterface
{
	GENERATED_BODY()
};

class G_LAB_API ITransitionModifier
{
	GENERATED_BODY()

public:

	virtual void Execute(UBaseAnimInstance* anim, FIKParams& currentParam, FTransitIKParams& transitParams) PURE_VIRTUAL(TEXT("NOT IMPLEMENTED YET"), return; );

};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class G_LAB_API UTransitionModifierAdditionalHeight : public UObject, public ITransitionModifier
{

	GENERATED_BODY()

public:

	virtual void Execute(UBaseAnimInstance* anim, FIKParams& currentParam, FTransitIKParams& transitParams) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName HeightCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HeightScale{ 8 };
};

USTRUCT(BlueprintType, Blueprintable)
struct FTransitIKParams 
{
	GENERATED_BODY()

public:

	FTransitIKParams() {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName IKName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName WeightTransitionCurveName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TargetWeight;

	UPROPERTY(BlueprintReadOnly)
	FVector InitialLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (MustImplement = "TransitionModifier"))
	TArray<TSubclassOf<UObject>> Modifier;

	UPROPERTY()
	TArray<ITransitionModifier*> ModifierInstances;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootCurveName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootBoneReference;

	UPROPERTY()
	FVector TransitLockLocation;

	UPROPERTY()
	FVector InitialRootLocation;

	UPROPERTY()
	FRotator InitialRootRotation;

	UPROPERTY(BlueprintReadWrite)
	FVector CurrentRootLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator CurrrentRootRotation;
};

/*************
*TURN IN PLACE
**************/
USTRUCT(BlueprintType, Blueprintable)
struct FTurnInPlaceParams
{
	GENERATED_BODY()

public:

	FTurnInPlaceParams() {};
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName RootBone;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EAxis::Type> Axis;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinDeviationToTrigger;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimSequenceBase* TurnAnim;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRuntimeFloatCurve IntensityCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRuntimeFloatCurve MaxVelocityByTurningRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float EndPaddingAnimTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PlayRate{1};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float StartingPointAnim;
};

/*****
* LEAN
******/
USTRUCT()
struct FLeanBone 
{
	GENERATED_BODY()
	
public:
	
	FLeanBone() {};

	FLeanBone(
		FName name,
		FTransform transform
	):	Name(name)
	,	Transform(transform){};

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	bool Dealocation{false};
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ULeanModifierParams : public UObject
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ULeanProceduralParams : public ULeanModifierParams
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName Root;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName Effector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRuntimeFloatCurve LeanIntensityCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxAdditiveAngle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxAdditiveDealocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Velocity{ 0.8 };

	UPROPERTY()
	TArray<FLeanBone> BoneChain;

	UPROPERTY()
	FRotator PreviewLeanAngle;

	UPROPERTY()
	FRotator PreviewDiffLeanAngle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EAxis::Type> AxisReference;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EAxis::Type> AxisEffective;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Dealocation{ false };
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ULeanBlendingAnimParams : public ULeanModifierParams
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRuntimeFloatCurve LeanIntensityCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EAxis::Type> AxisReference;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimSequenceBase* PositiveLeanAnim;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimSequenceBase* NegativeLeanAnim;

	UPROPERTY()
	float CurrentIntensity;



};

USTRUCT(BlueprintType, Blueprintable)
struct FLean 
{
	GENERATED_BODY()

public:

	FLean() {};

	FLean(TArray<FLeanBone> bones, bool dealocation)
	:	Bones(bones)
	,	Dealocation(dealocation)
	{};

	UPROPERTY()
	TArray<FLeanBone> Bones;

	UPROPERTY()
	bool Dealocation;
};

USTRUCT(BlueprintType, Blueprintable)
struct FLeanBlend
{
	GENERATED_BODY()

public:

	FLeanBlend(): IsNone(true) {}
	
	FLeanBlend(UAnimSequenceBase* anim, float intensity, EAxis::Type axis)
	:	LeanAnim(anim)
	,	Intensity(intensity)
	,	Axis(axis)
	,	IsNone(false)
	{};

	UPROPERTY()
	TEnumAsByte<EAxis::Type> Axis;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Intensity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimSequenceBase* LeanAnim;

	bool GetIsNone();

private:

	bool IsNone;

};


UCLASS()
class G_LAB_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	FVector GetDesiredDirection();

	/*****
	* IKs
	*****/
	UFUNCTION(BlueprintCallable)
	FIKData GetIKData(FIKParams& ikParams, bool& hitted);

	UFUNCTION(BlueprintCallable)
	TArray<FIKParams> UpdateIKs();

	void UpdateRoots();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|IKs")
	TMap<FName, FIKParams> IKParams;

	UFUNCTION(BlueprintCallable)
	void UpdateReverseMaskStartTraceLocation(FName ikName, FVector newLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	TArray<FIKParams> GetIKParamsValues();

	UFUNCTION(BlueprintCallable)
	void SetStopping(bool flag);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|IKs")
	TArray<FIKRoots> IKRoots;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|IKs")
	bool DebugIKs { false };

	/***************
	* VELOCITY STATS
	****************/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Movement")
	float IdleMoveThreshold;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsStopping;

	UPROPERTY(BlueprintReadOnly)
	bool IsDecelerating;

	UPROPERTY(BlueprintReadOnly)
	bool IsAccelerating;

	UFUNCTION(BlueprintCallable)
	void UpdateVelocityStats();

	UPROPERTY()
	FVector LastVelocity{ FVector::Zero() };

	/***********
	* TRANSITION
	************/
	UPROPERTY(BlueprintReadWrite)
	bool IsTransitioning;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool StoppingMovementAnimEnabled;

	UPROPERTY()
	TMap<FName, FTransitIKParams> IKTransitionInitialLocation;

	UFUNCTION(BlueprintCallable)
	virtual void SetInitialIKTransitions(TArray<FTransitIKParams> iksToTransit);

	UFUNCTION(BlueprintCallable)
	virtual void InterpolateIKTransition();

	UFUNCTION(BlueprintCallable)
	virtual void CleanIKTransitions();
	
	FVector GetRelativeIKLocation(FVector ikLocation);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Transition|Movement")
	bool MovingIdleTransitAnimEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Transition|IKs")
	bool DebugTransitionIKs{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Transition|IKs")
	bool OverrideRootDuringTransition;

	UPROPERTY(BlueprintReadWrite)
	FVector OverrideRootDuringTranitionLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator OverrideRootDuringTranitionRotator;

	/*****
	* LEAN
	******/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, Category = "Settings|Lean", meta = (AllowInherited = true))
	TArray<ULeanModifierParams*> LeanParams;
	/*UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Lean")
	TArray<FLeanParams> LeanParans;*/

	UFUNCTION(BlueprintCallable)
	bool SetupLean();

	bool SetupProceduralLean(ULeanProceduralParams* lean, ACharacter* charac);

	UFUNCTION(BlueprintCallable)
	void UpdateLean();

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	TArray<FLean> GetLeans();

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	TArray<FLeanBlend> GetBlendLeans();

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	FLeanBlend GetLeanBlendByAxis(TArray<FLeanBlend> leans, EAxis::Type axis);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (BlueprintThreadSafe))
	bool IsBlendLeanNone(FLeanBlend lean);

	/**************
	* TURN IN PLACE 
	***************/

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	TArray<FTurnInPlaceParams> TurnInPlaceParams;

	UPROPERTY(BlueprintReadWrite)
	UAnimSequenceBase* CurrentTurnInPlaceAnim;

	UPROPERTY(BlueprintReadWrite)
	float CurrentTurnInPlacePlayRate;

	UPROPERTY(BlueprintReadWrite)
	float CurrentTurnInPlaceStartOffset;

	UPROPERTY(BlueprintReadWrite)
	float CurrentTurnInPlaceMaxVelocity {1};

	UFUNCTION(BlueprintCallable)
	void UpdateTurnInplace(float DeltaTime);

	bool GetIsTurning();

protected:
	UPROPERTY(BlueprintReadOnly)
	bool IsTurning;

	UPROPERTY(BlueprintReadOnly)
	float CurrentTurningAnimLength;

	UPROPERTY(BlueprintReadOnly)
	float CurrentTurningProgression{0};

	UPROPERTY(BlueprintReadOnly)
	FRotator CurrentTurningDirection;

	UPROPERTY(BlueprintReadOnly)
	FRotator InitialtTurningDirection;

	UPROPERTY(BlueprintReadOnly)
	FRotator TargetTurningDirection;

	/*******
	*COMMONS
	********/
protected:
	
	float GetDeviationFromDesiredDirectionByAxis(
			ACharacter* charac
		,	EAxis::Type axisReference
		,	FRotator previewDiffAngleLerp
		,	float lerp
		,	FRotator& previewDesiredAngle
		,	FRotator& diffAngle 
	);

	FRotator GetForwardRotation(ACharacter* charac);
};
