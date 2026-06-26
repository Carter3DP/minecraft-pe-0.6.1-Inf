//
//  ShowKeyboardView.m
//  minecraftpe
//
//  Created by Johan Bernhardsson on 12/17/12.
//
//

#import "ShowKeyboardView.h"
#import "../../../src/platform/log.h"
#include "../../../src/platform/input/Keyboard.h"
#include <cstring>
#include <string>

NSString* const MCPEKeyboardSubmittedNotification = @"MCPEKeyboardSubmittedNotification";
NSString* const MCPEKeyboardCancelledNotification = @"MCPEKeyboardCancelledNotification";
NSString* const MCPEKeyboardSubmittedTextKey = @"text";

@implementation ShowKeyboardView

- (id)initWithFrame:(CGRect)frame {
    id returnId = [super initWithFrame:frame];
    textField = nil;
    lastString = @"";
    submittedText = [[NSMutableString alloc] init];

    UITapGestureRecognizer* tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(cancelInput)];
    tapRecognizer.cancelsTouchesInView = YES;
    [self addGestureRecognizer:tapRecognizer];
    [tapRecognizer release];

    self.backgroundColor = [UIColor clearColor];
    self.userInteractionEnabled = YES;
    return returnId;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [submittedText release];
    [textField release];
    [super dealloc];
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (BOOL)hasText {
    return YES;
}

- (void)insertText:(NSString *)text {
    const char* cText = [text cStringUsingEncoding:[NSString defaultCStringEncoding]];
    if (!cText)
        return;
    int strLength = strlen(cText);
    for (int a = 0; a < strLength; ++a) {
        if (cText[a] == 0 || cText[a] == '\n') {
            Keyboard::feed((char)Keyboard::KEY_RETURN, 1);
            Keyboard::feed((char)Keyboard::KEY_RETURN, 0);
            [self submitText];
            return;
        }

        [submittedText appendFormat:@"%c", cText[a]];
        Keyboard::feedText(cText[a]);
    }
}

- (void)deleteBackward {
    if ([submittedText length] > 0)
        [submittedText deleteCharactersInRange:NSMakeRange([submittedText length] - 1, 1)];
    Keyboard::feed((char)Keyboard::KEY_BACKSPACE, 1);
    Keyboard::feed((char)Keyboard::KEY_BACKSPACE, 0);
}

- (UIKeyboardType)keyboardType {
    return UIKeyboardTypeDefault;
}

- (UIReturnKeyType)returnKeyType {
    return UIReturnKeySend;
}

- (UITextAutocorrectionType)autocorrectionType {
    return UITextAutocorrectionTypeNo;
}

- (UITextAutocapitalizationType)autocapitalizationType {
    return UITextAutocapitalizationTypeNone;
}

- (void)showKeyboard {
    [submittedText setString:@""];
    [self becomeFirstResponder];
}

- (void)hideKeyboard {
    [self resignFirstResponder];
}

- (void)textFieldDidChange :(NSNotification *)notif {
}

- (BOOL)textFieldShouldReturn:(UITextField *)theTextField {
    Keyboard::feed((char)Keyboard::KEY_RETURN, 1);
    Keyboard::feed((char)Keyboard::KEY_RETURN, 0);
    [self submitText];
    return NO;
}

- (void)submitText {
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSString stringWithString:submittedText] forKey:MCPEKeyboardSubmittedTextKey];
    [[NSNotificationCenter defaultCenter] postNotificationName:MCPEKeyboardSubmittedNotification object:self userInfo:userInfo];
}

- (void)cancelInput {
    [self hideKeyboard];
    [[NSNotificationCenter defaultCenter] postNotificationName:MCPEKeyboardCancelledNotification object:self];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    [self cancelInput];
}

@end
