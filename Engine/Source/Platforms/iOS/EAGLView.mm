/******************************************************************************/
#include "iOS.h"
#undef super // Objective-C has its own 'super'

@interface GCController()
@property (nonatomic) unsigned long long deviceHash;
@end
/******************************************************************************/
Bool DontRemoveThisOrEAGLViewClassWontBeLinked;
/******************************************************************************/
namespace EE{
/******************************************************************************/
void (*ResizeAdPtr)();

static KB_KEY KeyMap[128];
static inline void SetKeyMap(Char c, KB_KEY key) {U16 u=c; if(InRange(u, KeyMap))KeyMap[u]=key;}
static void InitKeyMap()
{
   SetKeyMap('a', KB_A);
   SetKeyMap('A', KB_A);
   SetKeyMap('b', KB_B);
   SetKeyMap('B', KB_B);
   SetKeyMap('c', KB_C);
   SetKeyMap('C', KB_C);
   SetKeyMap('d', KB_D);
   SetKeyMap('D', KB_D);
   SetKeyMap('e', KB_E);
   SetKeyMap('E', KB_E);
   SetKeyMap('f', KB_F);
   SetKeyMap('F', KB_F);
   SetKeyMap('g', KB_G);
   SetKeyMap('G', KB_G);
   SetKeyMap('h', KB_H);
   SetKeyMap('H', KB_H);
   SetKeyMap('i', KB_I);
   SetKeyMap('I', KB_I);
   SetKeyMap('j', KB_J);
   SetKeyMap('J', KB_J);
   SetKeyMap('k', KB_K);
   SetKeyMap('K', KB_K);
   SetKeyMap('l', KB_L);
   SetKeyMap('L', KB_L);
   SetKeyMap('m', KB_M);
   SetKeyMap('M', KB_M);
   SetKeyMap('n', KB_N);
   SetKeyMap('N', KB_N);
   SetKeyMap('o', KB_O);
   SetKeyMap('O', KB_O);
   SetKeyMap('p', KB_P);
   SetKeyMap('P', KB_P);
   SetKeyMap('q', KB_Q);
   SetKeyMap('Q', KB_Q);
   SetKeyMap('r', KB_R);
   SetKeyMap('R', KB_R);
   SetKeyMap('s', KB_S);
   SetKeyMap('S', KB_S);
   SetKeyMap('t', KB_T);
   SetKeyMap('T', KB_T);
   SetKeyMap('u', KB_U);
   SetKeyMap('U', KB_U);
   SetKeyMap('v', KB_V);
   SetKeyMap('V', KB_V);
   SetKeyMap('w', KB_W);
   SetKeyMap('W', KB_W);
   SetKeyMap('x', KB_X);
   SetKeyMap('X', KB_X);
   SetKeyMap('y', KB_Y);
   SetKeyMap('Y', KB_Y);
   SetKeyMap('z', KB_Z);
   SetKeyMap('Z', KB_Z);

   SetKeyMap('1', KB_1);
   SetKeyMap('!', KB_1);
   SetKeyMap('2', KB_2);
   SetKeyMap('@', KB_2);
   SetKeyMap('3', KB_3);
   SetKeyMap('#', KB_3);
   SetKeyMap('4', KB_4);
   SetKeyMap('$', KB_4);
   SetKeyMap('5', KB_5);
   SetKeyMap('%', KB_5);
   SetKeyMap('6', KB_6);
   SetKeyMap('^', KB_6);
   SetKeyMap('7', KB_7);
   SetKeyMap('&', KB_7);
   SetKeyMap('8', KB_8);
   SetKeyMap('*', KB_8);
   SetKeyMap('9', KB_9);
   SetKeyMap('(', KB_9);
   SetKeyMap('0', KB_0);
   SetKeyMap(')', KB_0);

   SetKeyMap('-' , KB_SUB       );
   SetKeyMap('_' , KB_SUB       );
   SetKeyMap('=' , KB_EQUAL     );
   SetKeyMap('+' , KB_EQUAL     );
   SetKeyMap('[' , KB_LBRACKET  );
   SetKeyMap('{' , KB_LBRACKET  );
   SetKeyMap(']' , KB_RBRACKET  );
   SetKeyMap('}' , KB_RBRACKET  );
   SetKeyMap(';' , KB_SEMICOLON );
   SetKeyMap(':' , KB_SEMICOLON );
   SetKeyMap('\'', KB_APOSTROPHE);
   SetKeyMap('"' , KB_APOSTROPHE);
   SetKeyMap(',' , KB_COMMA     );
   SetKeyMap('<' , KB_COMMA     );
   SetKeyMap('.' , KB_DOT       );
   SetKeyMap('>' , KB_DOT       );
   SetKeyMap('/' , KB_SLASH     );
   SetKeyMap('?' , KB_SLASH     );
   SetKeyMap('\\', KB_BACKSLASH );
   SetKeyMap('|' , KB_BACKSLASH );
   SetKeyMap('`' , KB_TILDE     );
   SetKeyMap('~' , KB_TILDE     );

   SetKeyMap('\n', KB_ENTER);
   SetKeyMap('\t', KB_TAB  );
}
/******************************************************************************/
EAGLView* GetUIView()
{
   return ViewController ? (EAGLView*)ViewController.view : null;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
@implementation EAGLView
/******************************************************************************/
+(Class)layerClass
{
   return [CAEAGLLayer class];
}
/******************************************************************************/
-(id)initWithCoder:(NSCoder*)coder
{    
   if(self=[super initWithCoder:coder])
   {
      // Get the layer
      CAEAGLLayer *layer=(CAEAGLLayer*)self.layer;
      layer.opaque=true;
      layer.drawableProperties=[NSDictionary dictionaryWithObjectsAndKeys:
         [NSNumber numberWithBool:false], kEAGLDrawablePropertyRetainedBacking, // discard framebuffer contents after drawing frame, for better performance
         LINEAR_GAMMA ? kEAGLColorFormatSRGBA8 : kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
         nil];

      initialized=false;
      display_link=nil;
      InitKeyMap();

      self.multipleTouchEnabled=true;
      self.contentScaleFactor=ScreenScale;

      // setup notifications for the keyboard
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWasShown    :) name:UIKeyboardDidShowNotification  object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillBeHidden:) name:UIKeyboardWillHideNotification object:nil]; 

      // setup notifications for gamepads
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controllerWasConnected   :) name:GCControllerDidConnectNotification    object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controllerWasDisconnected:) name:GCControllerDidDisconnectNotification object:nil];
   }
   return self;
}
/******************************************************************************/
-(void)keyboardWasShown:(NSNotification*)notification
{
   Kb._visible=true;
   NSDictionary *info=[notification userInfo];
   CGRect rect=[[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
   RectI  recti(Round(rect.origin.x*ScreenScale), Round(rect.origin.y*ScreenScale), Round((rect.origin.x+rect.size.width)*ScreenScale), Round((rect.origin.y+rect.size.height)*ScreenScale));
   switch(App.orientation())
   {
      default       : Kb._recti=recti; break;
      case DIR_DOWN : Kb._recti.set(recti.min.x, D.resH()-recti.max.y, recti.max.x, D.resH()-recti.min.y); break;
      case DIR_RIGHT: Kb._recti.set(recti.min.y,          recti.min.x, recti.max.y,          recti.max.x); break;
      case DIR_LEFT : Kb._recti.set(recti.min.y, D.resH()-recti.max.x, recti.max.y, D.resH()-recti.min.x); break;
   }
}
-(void)keyboardWillBeHidden:(NSNotification*)notification {Kb._visible=false;}
-(void)keyboardVisible:(Bool)visible
{
   if(visible)[self becomeFirstResponder];else [self resignFirstResponder];
}
/******************************************************************************/
static Int Compare(C Joypad::Elm &a, C Joypad::Elm &b) {return ComparePtr(a.element, b.element);}
static Joypad* FindJoypad(GCExtendedGamepad *gamepad)
{
   REPA(Joypads){Joypad &jp=Joypads[i]; if(jp._gamepad==gamepad)return &jp;}
   return null;
}
-(void)controllerWasConnected:(NSNotification*)notification // these are called on the main thread
{
   GCController *controller=(GCController*)notification.object;
   if(GCExtendedGamepad *gamepad=controller.extendedGamepad)
   {
      ULong id=0; if([controller respondsToSelector:@selector(deviceHash)])id=controller.deviceHash;
      Bool added; Joypad &joypad=GetJoypad(NewJoypadID(id), added); if(added)
      {
         joypad._gamepad=gamepad;

       //controller.productCategory; "Xbox One"
         if([controller respondsToSelector:@selector(detailedProductCategory)])joypad._name=[controller detailedProductCategory]; // "Xbox Elite"
         if(!joypad._name.is())joypad._name=controller.vendorName; // "Xbox Wireless Controller"


         // add elements
         joypad.addPad(gamepad.dpad);

         joypad.addButton(gamepad.buttonA, JB_A);
         joypad.addButton(gamepad.buttonB, JB_B);
         joypad.addButton(gamepad.buttonX, JB_X);
         joypad.addButton(gamepad.buttonY, JB_Y);

         joypad.addButton(gamepad. leftShoulder, JB_L1);
         joypad.addButton(gamepad.rightShoulder, JB_R1);

         if(@available(macOS 10.14.1, iOS 12.1, tvOS 12.1, *))
         {
            joypad.addButton(gamepad. leftThumbstickButton, JB_L3);
            joypad.addButton(gamepad.rightThumbstickButton, JB_R3);
         }
         if(@available(macOS 10.15, iOS 13, tvOS 13, *))
         {
            joypad.addButton(gamepad.buttonOptions, JB_BACK);
            joypad.addButton(gamepad.buttonMenu   , JB_START);
         }

         joypad.addTrigger(gamepad. leftTrigger, 0);
         joypad.addTrigger(gamepad.rightTrigger, 1);

         joypad.addAxis(gamepad. leftThumbstick, 0);
         joypad.addAxis(gamepad.rightThumbstick, 1);

      #if 0
         if(@available(macOS 11, iOS 14, tvOS 14, *))
            if([gamepad isKindOfClass:[GCXboxGamepad class]])
         {
            GCXboxGamepad   *xbox_gamepad=(GCXboxGamepad*)gamepad;
            joypad.addButton(xbox_gamepad.paddleButton1, JB_PADDLE1);
            joypad.addButton(xbox_gamepad.paddleButton2, JB_PADDLE2);
            joypad.addButton(xbox_gamepad.paddleButton3, JB_PADDLE3);
            joypad.addButton(xbox_gamepad.paddleButton4, JB_PADDLE4);
         }
      #endif

         joypad._elms.sort(Compare);

         // set callback at the end
         gamepad.valueChangedHandler=^(GCExtendedGamepad *gamepad, GCControllerElement *element) {if(Joypad *joypad=FindJoypad(gamepad))joypad->changed(element);};
      }
   }
}
-(void)controllerWasDisconnected:(NSNotification*)notification // these are called on the main thread
{
   GCController *controller=(GCController*)notification.object;
   if(GCExtendedGamepad *gamepad=controller.extendedGamepad)Joypads.remove(FindJoypad(gamepad));
}
/******************************************************************************/
-(void)layoutSubviews // this is called when the layer is initialized, resized or a sub view is added (like ads)
{
   if(App._closed)return; // do nothing if app called 'Exit'
   // always proceed, because issues may appear when screen is rotated while full screen ad is displayed, and then we go back to the application
   if(!initialized)
   {
      initialized=true;
      if(!App.create())Exit("Failed to initialize the application"); // something failed
      [self setUpdate];
   }else
   {
      D._res.zero(); D.modeSet(1, 1); // clear current res to force setting mode according to newly detected resolution, for now use dummy values, actual values will be detected later
   }
   if(ResizeAdPtr)ResizeAdPtr(); // re-position banner after having everything, call this from a pointer to function (and not directly to the advertisement codes) to avoid force linking to advertisement (which then links to google admob lib, which uses 'advertisingIdentifier' which is prohibited when ads are not displayed in iOS, also linking to admob increases the app binary size)
}
/******************************************************************************/
-(void)update:(id)sender
{
   if(App._close)
   {
      App.del(); // manually call shut down
      ExitNow(); // force exit as iOS does not offer a clean way to do it
   }
   if(App.active())App.update();
}
/******************************************************************************/
-(void)setUpdate
{
   SyncLocker locker(D._lock); // thread safety because this method can be called by 'Exit'
   bool run =(initialized && App.active() && !App._closed); // don't run if app called 'Exit'
   if(  run!=(display_link!=null))
   {
      if(run)
      {
         display_link=[CADisplayLink displayLinkWithTarget:self selector:@selector(update:)];
       //display_link.preferredFramesPerSecond=..; this is not needed because default value is 0, which is max possible frame rate - https://developer.apple.com/documentation/quartzcore/cadisplaylink/1648421-preferredframespersecond?language=objc
         [display_link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
      }else
      {
         [display_link invalidate];
         display_link=nil;
      }
   }
}
/******************************************************************************/
-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	for(UITouch *touch in touches)
   {
      CGPoint pos=[touch locationInView:self];
      Vec2    p(pos.x, pos.y); p*=ScreenScale;
      VecI2   pi=Round(p); p=D.windowPixelToScreen(p);
      Touch  *t=FindTouchByHandle(touch); // find existing one
      if(!t) // create new one
      {
         t=&Touches.New().init(pi, p, touch, touch.type==UITouchTypeStylus);
      }else
      {
         t->_delta_pixeli_clp+=pi-t->_pixeli;
         t->_pixeli           =pi;
         t->_pos              =p;
      }
      Int taps =[touch tapCount]; // it starts from 1
      t->_first=(taps&1); // it's a first click on odd numbers, 1, 3, 5, ..
      t->_state=BS_ON|BS_PUSHED;
      if(!t->_first)t->_state|=BS_DOUBLE; // set double clicking only on even numbers, 2, 4, 6, ..
   }
}
-(void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
   for(UITouch *touch in touches)
   {
      CGPoint pos=[touch locationInView:self];
      Vec2    p(pos.x, pos.y); p*=ScreenScale;
      VecI2   pi=Round(p); p=D.windowPixelToScreen(p);
      Touch  *t=FindTouchByHandle(touch); // find existing one
      if(!t) // create new one
      {
         t=&Touches.New().init(pi, p, touch, touch.type==UITouchTypeStylus);
         t->_state=BS_ON|BS_PUSHED;
      }
      t->_delta_pixeli_clp+=pi-t->_pixeli;
      t->_pixeli           =pi;
      t->_pos              =p;
   }
}
-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
   for(UITouch *touch in touches)if(Touch *t=FindTouchByHandle(touch))
   {
      CGPoint pos=[touch locationInView:self];
      Vec2    p(pos.x, pos.y); p*=ScreenScale;
      VecI2   pi=Round(p); p=D.windowPixelToScreen(p);
      t->_delta_pixeli_clp+=pi-t->_pixeli;
      t->_pixeli           =pi;
      t->_pos              =p;
      t->_remove           =true;
      if(t->_state&BS_ON) // check for state in case it was manually eaten
      {
         t->_state|= BS_RELEASED;
         t->_state&=~BS_ON;
      }
   }
}
-(void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
   [self touchesEnded:touches withEvent:event];
}
/******************************************************************************/
-(void)insertText:(NSString*)text
{
   Str s=text; FREPA(s)
   {
      Char c=s[i]; Kb.queue(c, -1);
    //U16  u=c   ; if(InRange(u, KeyMap)){KB_KEY k=KeyMap[u]; Kb.push(k); Kb.release(k);}
   }
}
-(void)deleteBackward
{
   Kb.push   (KB_BACK, -1);
   Kb.release(KB_BACK);
}
-(BOOL)hasText                 {return true;}
-(BOOL)canBecomeFirstResponder {return true;}
/******************************************************************************/
@end
/******************************************************************************/
