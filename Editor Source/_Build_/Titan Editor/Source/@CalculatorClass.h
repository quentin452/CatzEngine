/******************************************************************************/
/******************************************************************************/
class CalculatorClass : ClosableWindow {
    TextLine expression;

    void visibleToggleActivate();

    CalculatorClass &create(C Rect &rect);
    virtual void update(C GuiPC &gpc) override;
    virtual void draw(C GuiPC &gpc) override;
};
/******************************************************************************/
/******************************************************************************/
extern CalculatorClass Calculator;
/******************************************************************************/
