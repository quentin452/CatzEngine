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
CChar* TAction();
CChar* TAmbientOcclusion();
CChar* TAmbientOcclusionRes();
CChar* TAmbientVolume();
CChar* TArachnids();
CChar* TArmor();
CChar* TArrow();
CChar* TAttack();
CChar* TAuthor();
CChar* TAxe();
CChar* TBack();
CChar* TBigAxe();
CChar* TBigHammer();
CChar* TBigSword();
CChar* TBlock();
CChar* TBlocked(TRANS_FORM form=TF_NEUTRAL);
CChar* TBlue();
CChar* TBolt();
CChar* TBow();
CChar* TBrightness();
CChar* TBumpMapping();
CChar* TBuy();
CChar* TCancel();
CChar* TCantDropItemHere();
CChar* TCantReadSpellBookNoMagic();
CChar* TCartoon();
CChar* TCastle();
CChar* TCastSpell();
CChar* TCave();
CChar* TCellar();
CChar* TChangeItem();
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
CChar* TController();
CChar* TControllerDeadZone();
CChar* TControllerInvert();
CChar* TControllerSensitivity();
CChar* TControllerVibration();
Str    TCreatedBy(CChar *name);
CChar* TCrossbow();
CChar* TCrouch();
CChar* TCrypt();
CChar* TCustom(TRANS_FORM form=TF_NEUTRAL);
CChar* TDash();
CChar* TDagger();
CChar* TDamage();
CChar* TDamageL();
CChar* TDamageR();
CChar* TDefault();
CChar* TDelete();
CChar* TDeviceVibration();
CChar* TDexterity();
CChar* TDifficulty();
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
CChar* TEditor();
CChar* TEgyptianTomb();
CChar* TEquip();
CChar* TExclusive();
CChar* TExit();
CChar* TExitToSystem();
CChar* TExp();
CChar* TExperience();
CChar* TEyeAdaptation();
CChar* TFeedback();
CChar* TFeedbackOK();
CChar* TFeedbackError();
CChar* TFieldOfView();
CChar* TFireball();
CChar* TFireBomb();
CChar* TFloorPlate();
CChar* TFor();
CChar* TForgottenTemple();
CChar* TFreeze();
CChar* TFreezeBomb();
CChar* TFullHealthReached();
CChar* TFullscreen();
CChar* TGamepad();
CChar* TGamepadDeadZone();
CChar* TGamepadInvert();
CChar* TGamepadSensitivity();
CChar* TGamepadVibration();
CChar* TGold();
CChar* TGround();
CChar* TGuiScale();
CChar* THammer();
CChar* THat();
CChar* THeal();
CChar* THealth();
CChar* THealthPotion();
CChar* THelmet();
CChar* THeroes();
CChar* THeroLevel();
CChar* THigh(TRANS_FORM form=TF_NEUTRAL);
CChar* THighPrecLight();
CChar* THighPrecLitCol();
CChar* TImprove();
CChar* TIncreasedSpellLevel();
CChar* TInvalidEmail();
CChar* TInventory();
CChar* TJump();
CChar* TKatana();
CChar* TKeep();
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
CChar* TLevel();
CChar* TLevels();
CChar* TLoading();
CChar* TLocked();
CChar* TTouchRotation();
CChar* TLow(TRANS_FORM form=TF_NEUTRAL);
CChar* TMace();
CChar* TMagic();
CChar* TMap();
CChar* TMana();
CChar* TManaPotion();
CChar* TMasterVolume();
CChar* TMayan();
CChar* TMaxLights();
CChar* TMedium(TRANS_FORM form=TF_NEUTRAL);
CChar* TMenu();
CChar* TMessage();
CChar* TMiniMap();
CChar* TMonster();
CChar* TMonsters();
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
CChar* TMultiSampling();
CChar* TMusicVolume();
CChar* TMute();
CChar* TMyMaps();
CChar* TName();
CChar* TNeedMoreGems();
CChar* TNeedMoreGold();
CChar* TNeedMoreMana();
CChar* TNeedMoreTime();
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
CChar* TObjects();
CChar* TOilBomb();
inline CChar* TOK() {return u"OK";}
CChar* TOnServer();
CChar* TOpen();
CChar* TOptions();
CChar* TPainted();
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
CChar* TPoints();
CChar* TPoisonBomb();
CChar* TPoisonCloud();
CChar* TPolearm();
CChar* TPotion();
CChar* TPoweredBy();
Str    TPoweredBy(CChar *name);
CChar* TPractice();
CChar* TPress();
CChar* TPrevious();
CChar* TPreviousItem();
CChar* TPreviousSpell();
CChar* TPrison();
CChar* TProp();
CChar* TPunch();
CChar* TPunchLeft();
CChar* TPunchRight();
CChar* TPutOnYourHeadset();
CChar* TPyramid();
CChar* TQuiver();
CChar* TRagePotion();
CChar* TRandom(TRANS_FORM form=TF_NEUTRAL);
CChar* TRanking();
CChar* TRead();
CChar* TRealistic();
CChar* TRecenterVR();
CChar* TRemove();
CChar* TRename();
CChar* TReport();
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
CChar* TStorage();
CChar* TStrength();
CChar* TSword();
CChar* TSynchronization();
CChar* TTakeAll();
CChar* TTakeAllItems();
CChar* TTeleport();
CChar* TTextureFiltering();
CChar* TTextureSize();
CChar* TThanksForPlaying();
CChar* TThrow();
CChar* TThrown();
CChar* TToggleFullscreen();
CChar* TToggleWeaponSet();
CChar* TToneMapping();
CChar* TTowers();
CChar* TTreasure();
CChar* TTurnLeft();
CChar* TTurnRight();
CChar* TTutorial();
CChar* TType();
CChar* TUnequip();
CChar* TUnlocked();
CChar* TUp();
CChar* TUpdated();
CChar* TUpload();
CChar* TUpscaleFiltering();
CChar* TUseItem();
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
CChar* TWouldYouLikeToChangeName();
CChar* TYes();
CChar* TYouHaveDied();
CChar* TYourEmail();
CChar* TZombies();
CChar* TZoom();
/******************************************************************************/
