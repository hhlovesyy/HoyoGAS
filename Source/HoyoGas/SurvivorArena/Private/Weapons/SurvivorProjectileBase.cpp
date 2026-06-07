#include "Weapons/SurvivorProjectileBase.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASurvivorProjectileBase::ASurvivorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	//UProjectileMovementComponent (绠€绉?PMC) 鏄櫄骞诲紩鎿庝腑涓撻棬鐢ㄤ簬澶勭悊闈炶鑹茬被銆佸叿鏈夊脊閬撶壒鎬х殑鐗╀綋绉诲姩鐨勫唴缃粍浠躲€傚畠缁ф壙鑷?UMovementComponent
	//绠€鍗曟潵璇达紝濡傛灉 CharacterMovementComponent 鏄笓闂ㄧ敤鏉ヤ己鍊欌€滈暱鐫€涓ゆ潯鑵裤€佸彈杈撳叆鎺у埗鐨勬椿浜衡€濈殑锛岄偅涔?ProjectileMovementComponent 灏辨槸涓撻棬鐢ㄦ潵浼哄€欌€滆鎵斿嚭鍘汇€侀潬鎯€у拰鐗╃悊瑙勫垯椋炶鐨勬鐗┾€濈殑銆?	//PMC 鍦ㄥ簳灞傛渶澶х殑浠峰€煎湪浜庡畠灏佽浜嗘瀬鍏朵弗璋ㄧ殑绉诲姩涓庣鎾炴竻绠楅€昏緫锛圫afeMoveUpdatedComponent锛夈€?	//褰撳瓙寮规瘡涓€甯у線鍓嶆帹杩涙椂锛孭MC 浼氬甫鐫€浣犳敞鍐岀殑閭ｄ釜鐪熷疄鐨?Collision Component 鍘诲仛鐗╃悊鎵ā锛圫weep锛夈€備竴鏃﹀湪杩欎竴甯х殑绉诲姩璺嚎涓婄鍒颁簡涓滆タ锛屽畠浼氳嚜鍔ㄨ绠楀嚭鍑讳腑鐐圭殑娉曠嚎锛圢ormal锛夈€佸墿浣欑殑椋炶鏃堕棿锛屽苟绮惧噯鍦板仠鍦ㄧ墿浣撹〃闈紝浠庤€岃Е鍙?OnComponentHit 鎴?OnComponentBeginOverlap锛岀粷瀵逛笉浼氬祵杩涘閲屻€?	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 0.0f;
	ProjectileMovementComponent->MaxSpeed = 0.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bAutoActivate = false;
	ProjectileMovementComponent->SetUpdatedComponent(SceneRoot);
}

void ASurvivorProjectileBase::SetPrimaryCollisionComponent(UPrimitiveComponent* InComponent)
{
	//杩欎釜鍩虹被涓嶅己鍒跺垱寤轰换浣曠壒瀹氱殑纰版挒浣擄紝鑰屾槸鎶?SceneRoot 浣滀负鏈€澶栧眰鏍硅妭鐐广€傚畠鏆撮湶浜?SetPrimaryCollisionComponent锛屽厑璁稿湪钃濆浘閲岄殢鎰忔坊鍔犵鎾炰綋锛堟瘮濡傚姞涓?Box锛夛紝鐒跺悗璋冪敤杩欎釜鑺傜偣鎶婂畠娉ㄥ唽涓轰富瑕佺殑鈥滅鎾炴劅搴斿櫒鈥濄€?	//瀹冭繕浼氳嚜鍔ㄥ府浣犲鐞嗕簨浠剁粦瀹氱殑鈥滄摝灞佽偂鈥濆伐浣滐紙绉婚櫎鏃х殑锛岀粦瀹氭柊鐨勶級锛屽苟涓斿姩鎬佹妸 ProjectileMovementComponent 鐨勬洿鏂扮洰鏍囪浆绉诲埌鏂扮殑纰版挒浣撲笂銆?	//SetUpdatedComponent锛氬湪 UE5 涓紝UProjectileMovementComponent (绠€绉?PMC) 鍏跺疄骞朵笉鐩存帴绉诲姩 AActor 鏈韩銆傚畠鍍忔槸涓€鍙扳€滅嫭绔嬬殑澶栨寕鍙戝姩鏈衡€濓紝浣犻渶瑕佹槑纭憡璇夎繖鍙板彂鍔ㄦ満锛氣€滀綘搴旇鎺ㄧ潃鍝釜杞︽灦瀛愯蛋锛熲€濃€斺€旇繖涓鎺ㄧ殑杞︽灦瀛愶紝灏辨槸 UpdatedComponent銆?	//榛樿鎯呭喌涓嬶紝PMC 浼氭帹鐫€ Actor 鐨勬牴缁勪欢 (RootComponent锛屽湪杩欓噷灏辨槸閭ｄ釜娌℃湁浠讳綍浣撶Н鐨勬暟瀛︾偣 SceneRoot) 绉诲姩銆備絾杩欐牱鍋氫細寮曞彂涓€涓瀬鍏惰嚧鍛界殑 Bug锛氬綋 PMC 鎺ㄧ潃 SceneRoot 杩涜楂橀€熺Щ鍔ㄦ椂锛屽簳灞傜殑鐗╃悊鎵ā锛圫weep锛変細浠?SceneRoot 鐨勫舰鐘讹紙涓€涓綋绉负 0 鐨勭偣锛夊幓杩涜纰版挒妫€娴嬨€傝繖浼氬鑷村瓙寮规瀬鏄撶┛閫忚杽澧欙紙杩欏氨鏄憲鍚嶇殑 Tunneling 绌挎ā鏁堝簲锛夛紝鍝€曞瓙寮圭殑瀛愬眰绾ф寕鐫€涓€涓法澶х殑 Box 纰版挒浣擄紝鍥犱负鐗╃悊寮曟搸鍦?Sweep 鏃舵牴鏈笉鐪嬪瓙缁勪欢鐨勪綋绉€?	//鎵€浠ワ紝ProjectileMovementComponent->SetUpdatedComponent(PrimaryCollisionComponent); 杩欏彞璇濈殑鎰忔€濇槸锛氣€滄妸鍙戝姩鏈虹殑鍙楀姏鐐癸紝浠庢病鏈変綋绉殑鏍硅妭鐐癸紝杞Щ鍒扮湡姝ｇ殑纰版挒浣撹韩涓娿€傗€?杩欐牱锛岀墿鐞嗗紩鎿庡氨浼氱敤杩欎釜纰版挒浣撶殑瀹為檯闀垮楂樼瓑鍖呭洿鐩掓暟鎹幓杩涜鎵ā妫€娴嬶紝纭繚鏋侀€熼琛屾椂缁濆涓嶄細婕忓垽銆?	
	// 1. 鐩稿悓缁勪欢鐩存帴鎷︽埅锛堥伩鍏嶉噸澶嶇粦瀹氾級
	if (PrimaryCollisionComponent == InComponent)
	{
		return;
	}

	// 2. 鍓ョ鏃х姸鎬侊紙娓呯悊鐜板満锛?	if (PrimaryCollisionComponent)
	if (PrimaryCollisionComponent)
	{
		PrimaryCollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ASurvivorProjectileBase::OnPrimaryCollisionBeginOverlap);
	}

	// 3. 鐘舵€佹洿鏂?	PrimaryCollisionComponent = InComponent;
	PrimaryCollisionComponent = InComponent;
	USceneComponent* NewMoveTarget = SceneRoot; // 鍏滃簳绛栫暐锛氬鏋滄病纰版挒浣擄紝鍙戝姩鏈哄氨閫€鍖栧幓鎺?Root

	// 4. 缁戝畾鏂扮姸鎬?	if (PrimaryCollisionComponent)
	if (PrimaryCollisionComponent)
	{
		PrimaryCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASurvivorProjectileBase::OnPrimaryCollisionBeginOverlap);
		NewMoveTarget = PrimaryCollisionComponent; // 鏈夌湡瀹炵鎾炰綋锛屽氨璁╁彂鍔ㄦ満鎺ㄧ湡瀹炵鎾炰綋
	}

	// 5. 缁熶竴鏇存柊鍙戝姩鏈虹殑椹卞姩鐩爣
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->SetUpdatedComponent(NewMoveTarget);
	}
}

void ASurvivorProjectileBase::InitializeProjectile(
	UAbilitySystemComponent* InSourceASC,
	const FGameplayEffectSpecHandle& InDamageSpecHandle,
	const FVector& Direction,
	float Speed,
	float LifeSeconds)
{
	//鐜╁閲婃斁鎶€鑳斤紙Gameplay Ability锛夋椂锛岀畻鍑轰激瀹筹紙姣斿鍩虹 10 鐐?+ 50% 鏀诲嚮鍔涳級锛屾妸杩欎釜绠楀ソ鐨勪激瀹虫墦鍖呮垚涓€涓寘瑁癸紙FGameplayEffectSpecHandle锛夈€?	//璋冪敤 InitializeProjectile 鏃讹紝鎶婅繖涓寘瑁癸紙SpecHandle锛夊拰鏄皝鍙戝嚭鐨勶紙SourceASC锛夊缁欐姇灏勭墿銆?	//绛炬敹锛?鎶曞皠鐗╁湪澶╀笂椋烇紝鍙鎾炲埌浜嗘晫浜猴紝瑙﹀彂 ApplyDamageSpecToActor銆傚畠瀹屽叏涓嶇浼ゅ鏄灏戯紝鐩存帴鎶婃€€閲岀殑鍖呰９濉炶繘鏁屼汉鐨?TargetASC 閲屻€傝繖鏍凤紝鎵€鏈夌殑浼ゅ淇銆佹毚鍑昏绠楅€昏緫閮藉湪鍙戝皠鑰呰韩涓婂畬鎴愪簡锛屾姇灏勭墿绫绘瀬鍏跺共鍑€銆?	SourceASC = InSourceASC;
	SourceASC = InSourceASC;
	DamageEffectSpecHandle = InDamageSpecHandle;
	HitActors.Reset();
	bProjectileInitialized = true;

	if (!PrimaryCollisionComponent)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Projectile %s has no PrimaryCollisionComponent. Assign a collision primitive and call SetPrimaryCollisionComponent before firing."), *GetNameSafe(this));
	}

	FVector SafeDirection = Direction;
	SafeDirection.Z = Direction.Z;
	if (!SafeDirection.Normalize())
	{
		SafeDirection = FVector::ForwardVector;
	}

	SetActorRotation(SafeDirection.Rotation());

	if (ProjectileMovementComponent)
	{
		const float SafeSpeed = FMath::Max(0.0f, Speed);
		ProjectileMovementComponent->SetUpdatedComponent(PrimaryCollisionComponent ? PrimaryCollisionComponent : SceneRoot);
		ProjectileMovementComponent->Velocity = SafeDirection * SafeSpeed;
		ProjectileMovementComponent->InitialSpeed = SafeSpeed;
		ProjectileMovementComponent->MaxSpeed = SafeSpeed;
		ProjectileMovementComponent->UpdateComponentVelocity();
		ProjectileMovementComponent->Activate(true);
	}

	SetLifeSpan(FMath::Max(0.0f, LifeSeconds));

	UE_LOG(LogSurvivorArena, Log, TEXT("Projectile initialized. Projectile=%s SourceASC=%s Speed=%.2f LifeSeconds=%.2f SpecValid=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InSourceASC),
		Speed,
		LifeSeconds,
		InDamageSpecHandle.IsValid() ? TEXT("true") : TEXT("false"));
}

UPrimitiveComponent* ASurvivorProjectileBase::GetPrimaryCollisionComponent() const
{
	return PrimaryCollisionComponent;
}

UProjectileMovementComponent* ASurvivorProjectileBase::GetProjectileMovementComponent() const
{
	return ProjectileMovementComponent;
}

void ASurvivorProjectileBase::HandleProjectileOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bProjectileInitialized)
	{
		return;
	}

	if (!CanHitActor(OtherActor))
	{
		return;
	}

	if (!bAllowMultipleHits && HitActors.Contains(OtherActor))
	{
		return;
	}

	HandleProjectileImpact(OtherActor, SweepResult);
}

void ASurvivorProjectileBase::HandleProjectileImpact(AActor* HitActor, const FHitResult& Hit)
{
	const bool bAppliedDamage = ApplyDamageSpecToActor(HitActor);
	if (!bAppliedDamage)
	{
		return;
	}

	HitActors.Add(HitActor); //杩欎釜鏈哄埗鏄仛闃叉涓€涓鑹茶В绠楀娆＄鎾炰激瀹崇殑锛岄槻姝㈡姈鍔?

	if (bDestroyOnHit)
	{
		Destroy();
	}
}

bool ASurvivorProjectileBase::CanHitActor(AActor* OtherActor) const
{
	if (!OtherActor || OtherActor == this)
	{
		return false;
	}
	//鍏充簬Instigator鐨勮鏄庯細鍦ㄨ櫄骞诲紩鎿庯紙UE5锛変腑锛孖nstigator 鏄竴涓潪甯搁噸瑕佷笖搴曞眰鐨勬牳蹇冩蹇点€傚瓧闈㈡剰鎬濇槸鈥滅吔鍔ㄨ€呪€濇垨鈥滃浣滀繎鑰呪€濓紝鍦ㄥ紩鎿庣殑 gameplay 鏋舵瀯涓紝瀹冪壒鎸囪Е鍙戝綋鍓嶈涓恒€侀噴鏀惧綋鍓嶆妧鑳芥垨閫犳垚褰撳墠浼ゅ鐨勯偅涓牳蹇冭鑹插疄浣擄紙Pawn / Character锛夈€?	//鐞嗚В瀹冧笌 Owner锛堟嫢鏈夎€咃級鐨勫尯鍒€傝繖鏄櫄骞诲紩鎿庝负浜嗚В鑰︹€滅墿鐞嗗眰绾р€濆拰鈥滈€昏緫璐ｄ换鈥濊€岃璁＄殑鍙岃建鍒躲€?	//姣斿璇寸帺瀹讹紙Actor锛夊紑鏋紝瀛愬脊鐨凮wner鏄灙锛屼絾鏄疘nstigator璧嬪€间负鐜╁锛岃繖鏍峰瓙寮规棦涓嶈兘瀵规灙閫犳垚浼ゅ锛屽綋鐒朵篃涓嶈兘瀵圭帺瀹堕€犳垚浼ゅ锛?
	if (OtherActor == GetInstigator() || OtherActor == GetOwner())
	{
		return false;
	}

	return true;
}

bool ASurvivorProjectileBase::ApplyDamageSpecToActor(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return false;
	}

	if (!SourceASC.IsValid())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Projectile %s cannot apply damage because SourceASC is null."), *GetNameSafe(this));
		return false;
	}

	if (!DamageEffectSpecHandle.IsValid() || !DamageEffectSpecHandle.Data.IsValid())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Projectile %s cannot apply damage because DamageEffectSpecHandle is invalid."), *GetNameSafe(this));
		return false;
	}

	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!TargetASI)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return false;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);

	UE_LOG(LogSurvivorArena, Log, TEXT("Projectile %s applied damage spec to %s"), *GetNameSafe(this), *GetNameSafe(TargetActor));
	return true;
}

void ASurvivorProjectileBase::OnPrimaryCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleProjectileOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}




