/******************************************************************************/
struct LangStr : Str // String for a specific language
{
   LANG_TYPE lang=LANG_NONE;

   void operator=(C Str &s) {super::operator=(s);}

   Bool save(File &f)C;
   Bool load(File &f) ;
};
struct MultiStr : Mems<LangStr> // Multi-Language String
{
   MultiStr& clear() {super::clear(); return T;}
   MultiStr& del  () {super::del  (); return T;}

   operator C Str&           (                        )C {return T();} // get    text for Application language, "" on fail
            C Str& operator()(                        )C;              // get    text for Application language, "" on fail
            C Str& operator()(LANG_TYPE lang          );               // get    text for 'lang'      language, "" on fail
         MultiStr& set       (LANG_TYPE lang, C Str &s);               // set    text for 'lang'      language
         MultiStr& remove    (LANG_TYPE lang          );               // remove text for 'lang'      language

   Bool find(LANG_TYPE lang, Int &index)C;
};
/******************************************************************************/
enum TRANS_FORM : Byte // Translation Form
{
   TF_MALE   , // masculine
   TF_FEMALE , // feminine
   TF_NEUTRAL, // neuter
};
/******************************************************************************/
CChar* T1Player();
CChar* T2Players();
CChar* TAbout();
CChar* TAction();
CChar* TAddImage();
CChar* TAddYouTubeVideo();
CChar* TAlways();
CChar* TAmbientOcclusion();
CChar* TAmbientOcclusionRes();
CChar* TAmbientVolume();
CChar* TApril();
CChar* TAprilShort();
CChar* TArachnids();
CChar* TAreYouSureYouWantToBlockThisUser();
CChar* TAreYouSureYouWantToChange();
CChar* TAreYouSureYouWantToDeleteThisComment();
CChar* TAreYouSureYouWantToDeleteThisPost();
CChar* TAreYouSureYouWantToDiscardChanges();
CChar* TAreYouSureYouWantToDiscardThisComment();
CChar* TAreYouSureYouWantToDiscardThisPost();
CChar* TAreYouSureYouWantToUnblockThisUser();
CChar* TArmor();
CChar* TArrow();
CChar* TAttack();
CChar* TAugust();
CChar* TAugustShort();
CChar* TAuthor();
CChar* TAutoAdjust();
CChar* TAxe();
CChar* TBack();
CChar* TBigAxe();
CChar* TBigHammer();
CChar* TBigSword();
CChar* TBirthdate();
CChar* TBlock();
CChar* TBlocked(TRANS_FORM form=TF_NEUTRAL);
CChar* TBlocking(TRANS_FORM form=TF_NEUTRAL);
CChar* TBlockUser();
CChar* TBlockVerb();
CChar* TBlood();
CChar* TBlue();
CChar* TBolt();
CChar* TBow();
CChar* TBrightness();
CChar* TBumpMapping();
CChar* TBuy();
CChar* TCancel();
CChar* TCantConnect();
CChar* TCantDropItemHere();
CChar* TCantImportImage();
CChar* TCantReadSpellBookNoMagic();
CChar* TCartoon();
CChar* TCastle();
CChar* TCastSpell();
CChar* TCave();
CChar* TCellar();
CChar* TChangeItem();
CChar* TChangePassword();
CChar* TChangeSpell();
CChar* TCharacterScreen();
CChar* TChest();
CChar* TChestKey();
CChar* TClass();
CChar* TClick();
CChar* TClose();
CChar* TCloseActiveWindow();
CChar* TClub();
CChar* TCollected();
CChar* TConfigureControllers();
CChar* TCongratulations();
CChar* TContinue();
CChar* TController();
CChar* TControllerDeadZone();
CChar* TControllerDisconnected();
CChar* TControllerInvert();
CChar* TControllerSensitivity();
CChar* TControllerVibration();
CChar* TControls();
CChar* TCopyCommentLink();
CChar* TCopyPostLink();
CChar* TCopyUserLink();
CChar* TCountry();
Str    TCreatedBy(CChar *name);
CChar* TCreateMods();
Str    TCredits();
CChar* TCrossbow();
CChar* TCrouch();
CChar* TCrypt();
CChar* TCustom(TRANS_FORM form=TF_NEUTRAL);
CChar* TDagger();
CChar* TDamage();
CChar* TDamageL();
CChar* TDamageR();
CChar* TDark(TRANS_FORM form=TF_NEUTRAL);
CChar* TDash();
CChar* TDay();
CChar* TDecember();
CChar* TDecemberShort();
CChar* TDefault();
CChar* TDelete();
CChar* TDeleteComment();
CChar* TDeletePost();
CChar* TDescription();
CChar* TDeviceVibration();
CChar* TDexterity();
CChar* TDifficulty();
CChar* TDiscard();
CChar* TDoor();
CChar* TDoorKey();
CChar* TDown();
CChar* TDownload();
CChar* TDownloaded();
CChar* TDownloads();
CChar* TDrawn();
CChar* TDrink();
CChar* TDrop();
CChar* TDungeon();
CChar* TDwarvenMine();
CChar* TEasy(TRANS_FORM form=TF_NEUTRAL);
CChar* TEdgeSoftening();
CChar* TEdit();
CChar* TEditProfile();
CChar* TEditor();
CChar* TEgyptianTomb();
CChar* TEmail();
CChar* TEmailAlreadyUsed();
CChar* TError();
CChar* TEquip();
CChar* TExclusive();
CChar* TExit();
CChar* TExitToSystem();
CChar* TExp();
CChar* TExperience();
CChar* TEyeAdaptation();
CChar* TFebruary();
CChar* TFebruaryShort();
CChar* TFeedback();
CChar* TFeedbackOK();
CChar* TFeedbackError();
CChar* TFemale();
CChar* TFieldOfView();
CChar* TFind();
CChar* TFireball();
CChar* TFireBomb();
CChar* TFloorPlate();
CChar* TFor();
CChar* TForgotPassword();
CChar* TForgottenTemple();
CChar* TFreeze();
CChar* TFreezeBomb();
CChar* TFullHealthReached();
CChar* TFullscreen();
CChar* TGamepad();
CChar* TGamepadDeadZone();
CChar* TGamepadDisconnected();
CChar* TGamepadInvert();
CChar* TGamepadSensitivity();
CChar* TGamepadVibration();
CChar* TGender();
CChar* TGeneral();
CChar* TGold();
CChar* TGraphics();
CChar* TGround();
CChar* TGuiScale();
CChar* THammer();
CChar* THat();
CChar* THeader();
CChar* THeal();
CChar* THealth();
CChar* THealthPotion();
CChar* THelmet();
CChar* THeroes();
CChar* THeroLevel();
CChar* THigh(TRANS_FORM form=TF_NEUTRAL);
CChar* THighPrecLight();
CChar* THighPrecLitCol();
inline CChar* TID() {return u"ID";}
CChar* TIDAlreadyUsed();
CChar* TImage();
CChar* TImprove();
CChar* TIncreasedSpellLevel();
CChar* TInstallMods();
CChar* TInvalidCountry();
CChar* TInvalidEmail();
CChar* TInvalidID();
CChar* TInvalidName();
CChar* TInvalidPassword();
CChar* TInvalidUser();
CChar* TInvalidYouTubeVideoLink();
CChar* TInventory();
CChar* TJanuary();
CChar* TJanuaryShort();
CChar* TJuly();
CChar* TJulyShort();
CChar* TJump();
CChar* TJune();
CChar* TJuneShort();
CChar* TKatana();
CChar* TKeep();
CChar* TKeepEditing();
CChar* TKey();
CChar* TKeyboard();
CChar* TKick();
CChar* TKickLow();
CChar* TKickHigh();
CChar* TKilledMonsters();
CChar* TKnife();
CChar* TKnockout();
CChar* TLAN();
CChar* TLanguage();
CChar* TLarge(TRANS_FORM form=TF_NEUTRAL);
CChar* TLearn();
CChar* TLearnedSpell();
CChar* TLeft();
CChar* TLegal();
CChar* TLevel();
CChar* TLevels();
CChar* TLoading();
CChar* TLocked();
CChar* TLogIn();
CChar* TLogOut();
CChar* TLow(TRANS_FORM form=TF_NEUTRAL);
CChar* TLoweredDifficulty();
CChar* TMace();
CChar* TMagic();
CChar* TMale();
CChar* TMap();
CChar* TMana();
CChar* TManaPotion();
CChar* TMarch();
CChar* TMarchShort();
CChar* TMasterVolume();
CChar* TMay();
CChar* TMayShort();
CChar* TMayan();
CChar* TMaxLights();
CChar* TMedium(TRANS_FORM form=TF_NEUTRAL);
CChar* TMenu();
CChar* TMessage();
CChar* TMiniMap();
CChar* TMods();
CChar* TModsFolder();
CChar* TMonster();
CChar* TMonsters();
CChar* TMonthName     (Int month); // get month full  name (January, February, March, ..), 'month'=1..12, null on fail
CChar* TMonthNameShort(Int month); // get month short name (Jan    , Feb     , Mar  , ..), 'month'=1..12, null on fail
CChar* TMoreManaNeeded();
CChar* TMotionBlur();
CChar* TMotionBlurIntensity();
CChar* TMouse();
CChar* TMouseInvert();
CChar* TMouseSensitivity();
CChar* TMove();
CChar* TMoveBack();
CChar* TMoveForward();
CChar* TMoveLeft();
CChar* TMoveRight();
CChar* TMoves();
CChar* TMultiSampling();
CChar* TMusicVolume();
CChar* TMute();
CChar* TMyMaps();
CChar* TName();
CChar* TNeedMoreGems();
CChar* TNeedMoreGold();
CChar* TNeedMoreMana();
CChar* TNeedMoreTime();
CChar* TNever();
CChar* TNew();
CChar* TNewMap();
CChar* TNext();
CChar* TNextItem();
CChar* TNextSpell();
CChar* TNo();
CChar* TNoAmmo();
CChar* TNoHeroSelected();
CChar* TNoMapSelected();
CChar* TNone(TRANS_FORM form=TF_NEUTRAL);
CChar* TNormal(TRANS_FORM form=TF_NEUTRAL);
CChar* TNovember();
CChar* TNovemberShort();
CChar* TNowUnderMaintenancePleaseTryAgainLater();
CChar* TObjects();
CChar* TOctober();
CChar* TOctoberShort();
CChar* TOilBomb();
inline CChar* TOK() {return u"OK";}
CChar* TOnServer();
CChar* TOpen();
CChar* TOpenModsFolder();
CChar* TOpponents();
CChar* TOptional(TRANS_FORM form=TF_NEUTRAL);
CChar* TOptions();
CChar* TPainted();
CChar* TPassword();
CChar* TPetHealth();
CChar* TPetLevelUp();
CChar* TPetScreen();
CChar* TPets();
CChar* TPickedUp();
CChar* TPit();
CChar* TPixelDensity();
CChar* TPlaceInsert();
CChar* TPlay();
CChar* TPlayer();
CChar* TPlayerLevel();
CChar* TPlayerLevelUp();
CChar* TPlayerScreen();
CChar* TPleaseCheckYourEmail();
CChar* TPleaseCheckYourEmailForVerificationLink();
CChar* TPleaseEnterYouTubeVideoLink();
CChar* TPleaseSpecifyTheProblem();
CChar* TPleaseRegister();
CChar* TPleaseVerifyYourEmailFirst();
CChar* TPoints();
CChar* TPoisonBomb();
CChar* TPoisonCloud();
CChar* TPolearm();
CChar* TPost();
CChar* TPostVerb();
CChar* TPostHasBeenDeleted();
CChar* TPostHasBeenReported();
CChar* TPostLinkHasBeenCopiedToClipboard();
CChar* TPotion();
CChar* TPoweredBy();
Str    TPoweredBy(CChar *name);
CChar* TPractice();
CChar* TPress();
CChar* TPrevious();
CChar* TPreviousItem();
CChar* TPreviousSpell();
CChar* TPrison();
CChar* TProfile();
CChar* TProp();
CChar* TPunch();
CChar* TPunchLeft();
CChar* TPunchRight();
CChar* TPutOnYourHeadset();
CChar* TPyramid();
CChar* TQuiver();
CChar* TRagePotion();
CChar* TRaisedDifficulty();
CChar* TRandom(TRANS_FORM form=TF_NEUTRAL);
CChar* TRanking();
CChar* TRead();
CChar* TRealistic();
CChar* TRecenterVR();
CChar* TRegister();
CChar* TRemove();
CChar* TRename();
CChar* TReport();
CChar* TReportPost();
CChar* TReportUser();
CChar* TReset();
CChar* TResolution();
CChar* TRestart();
CChar* TRevive();
CChar* TRevived();
CChar* TRight();
CChar* TRing();
CChar* TRooms();
CChar* TRotateLeft();
CChar* TRotateRight();
CChar* TScreenControls();
CChar* TScorpions();
CChar* TScythe();
CChar* TSecretWall();
CChar* TSelect();
CChar* TSelectAll();
CChar* TSelectAmount();
CChar* TSelectGraphicsStyle();
CChar* TSelectHero();
CChar* TSelectName();
CChar* TSelectPet();
CChar* TSelectYourHero();
CChar* TSell();
CChar* TSend();
CChar* TSending();
CChar* TSeptember();
CChar* TSeptemberShort();
CChar* TSettings();
CChar* TShadows();
CChar* TShadowsResolution();
CChar* TShadowsSofting();
CChar* TShadowsDither();
CChar* TSharpen();
CChar* TShield();
CChar* TShieldPotion();
CChar* TShop();
CChar* TShowHints();
CChar* TShowPlayerLevel();
CChar* TSickle();
CChar* TSize();
CChar* TSmall(TRANS_FORM form=TF_NEUTRAL);
CChar* TSnakes();
CChar* TSoftParticles();
CChar* TSomeoneAwesome();
CChar* TSometimes();
CChar* TSound();
CChar* TSoundVolume();
CChar* TSpear();
CChar* TSpecialThanksTo();
CChar* TSpeed();
CChar* TSpellBook();
CChar* TSpiders();
CChar* TStaff();
CChar* TStairsDown();
CChar* TStairsUp();
CChar* TStance();
CChar* TStatusBar();
CChar* TStorage();
CChar* TStrength();
CChar* TSuccess();
CChar* TSword();
CChar* TSynchronization();
CChar* TTakeAll();
CChar* TTakeAllItems();
CChar* TTeleport();
CChar* TTermsOfUse();
CChar* TTextureFiltering();
CChar* TTextureSize();
CChar* TThanksForPlaying();
CChar* TThankYou();
CChar* TThrow();
CChar* TThrown();
CChar* TToggleFullscreen();
CChar* TToggleWeaponSet();
CChar* TToneMapping();
CChar* TTouchRotation();
CChar* TTowers();
CChar* TTreasure();
CChar* TTurnLeft();
CChar* TTurnRight();
CChar* TTutorial();
CChar* TType();
CChar* TUnblock();
CChar* TUnblockUser();
CChar* TUnequip();
CChar* TUniqueID();
CChar* TUnknownError();
CChar* TUnlocked();
CChar* TUnlockedCharacter();
CChar* TUnsupportedFileType();
CChar* TUp();
CChar* TUpdated();
CChar* TUpload();
CChar* TUpscaleFiltering();
CChar* TUseItem();
CChar* TUserHasBeenBlocked();
CChar* TUserHasBeenReported();
CChar* TUserHasBeenUnblocked();
CChar* TUserLinkHasBeenCopiedToClipboard();
CChar* TUserNotFound();
CChar* TVersion();
CChar* TVeryHigh(TRANS_FORM form=TF_NEUTRAL);
CChar* TVitality();
CChar* TWakizashi();
CChar* TWalk();
CChar* TWallLever();
CChar* TWallText();
CChar* TWallTurret();
CChar* TWand();
CChar* TWeaponSets();
CChar* TWhatsTheProblem();
CChar* TWorld();
CChar* TWouldYouLikeToChangeName();
CChar* TWrongPassword();
CChar* TYear();
CChar* TYes();
CChar* TYouDontHavePermission();
CChar* TYouHaveDied();
CChar* TYourAccountIsSuspended();
CChar* TYourEmail();
CChar* TYoureUsingOutdatedApplicationVersionPleaseUpdate();
CChar* TZombies();
CChar* TZoom();
/******************************************************************************/
