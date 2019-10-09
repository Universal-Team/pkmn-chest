#ifndef LOADING_HPP
#define LOADING_HPP
extern int angle;
extern int angleChange;

// Set this as IRQ_VBLANK to spin the chest
void loadingAnimation(void);

// Loads the spinning chest
void loadLoadingLogo(void);

// Shows the spinning chest
void showLoadingLogo(void);

// Hides the spinning chest
void hideLoadingLogo(void);

#endif
