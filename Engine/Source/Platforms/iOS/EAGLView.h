/******************************************************************************/
@interface EAGLView : UIView <UIKeyInput> {
    Bool initialized;
    CADisplayLink *display_link;
}

- (void)update:(id)sender;
- (void)setUpdate;
- (void)keyboardVisible:(Bool)visible;

@end
/******************************************************************************/
namespace EE {
/******************************************************************************/
extern void (*ResizeAdPtr)();

EAGLView *GetUIView();
/******************************************************************************/
} // namespace EE
/******************************************************************************/
