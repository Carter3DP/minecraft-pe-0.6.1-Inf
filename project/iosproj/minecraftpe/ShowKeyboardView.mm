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
#include <string>

NSString* const MCPEKeyboardSubmittedNotification = @"MCPEKeyboardSubmittedNotification";
NSString* const MCPEKeyboardSubmittedTextKey = @"text";

@implementation ShowKeyboardView

- (id)initWithFrame:(CGRect)frame {
    id returnId = [super initWithFrame:frame];
    textField = [[UITextField alloc] init];
    submittedText = [[NSMutableString alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(textFieldDidChange:) name:UITextFieldTextDidChangeNotification object:textField];
    [textField setDelegate:self];
    [self addSubview:textField];
    textField.text = lastString = @"AAAAAAAAAAAAAAAAAAAA";
    return returnId;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [submittedText release];
    [textField release];
    [super dealloc];
}

/*- (void)insertText:(NSString *)text {
    const char* cText = [text cStringUsingEncoding:[NSString defaultCStringEncoding]];
    //std::string textString([text cStringUsingEncoding:[NSString defaultCStringEncoding]]);
    int strLength = strlen(cText);
    for(int a = 0; a < strLength; ++a) {
        LOGW("NewCharInput: %c (%d)\n", cText[a], cText[a]);
        if(cText[a] == 0 || cText[a] == '\n') {
            Keyboard::feed((char)Keyboard::KEY_RETURN, 1);
            Keyboard::feed((char)Keyboard::KEY_RETURN, 0);
        } else {
            Keyboard::feedText(cText[a]);
        }
    }
    //delete cText;
    //LOGW("Insert text: %s\n", [text cStringUsingEncoding:[NSString defaultCStringEncoding]]);
}

- (void)deleteBackward {
    //LOGW("deleteBackward\n");
    //Keyboard::feed((char)Keyboard::KEY_BACKSPACE, 1);
    //Keyboard::feed((char)Keyboard::KEY_BACKSPACE, 0);§
}*/

- (BOOL)hasText {
    return YES;
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)showKeyboard {
    [submittedText setString:@""];
    //[self becomeFirstResponder];
    [textField becomeFirstResponder];
    //[self becomeFirstResponder];
}

- (void)hideKeyboard {
    [textField resignFirstResponder];
    [self resignFirstResponder];
}

- (void)textFieldDidChange :(NSNotification *)notif {
    UITextField* txt = (UITextField*)notif.object;
    if(![txt.text isEqualToString:lastString]) {
        if(lastString.length > txt.text.length) {
            if ([submittedText length] > 0)
                [submittedText deleteCharactersInRange:NSMakeRange([submittedText length] - 1, 1)];
            Keyboard::feed((char)Keyboard::KEY_BACKSPACE, 1);
            Keyboard::feed((char)Keyboard::KEY_BACKSPACE, 0);
        } else if([txt.text characterAtIndex:(txt.text.length - 1)] == '\n') {
            Keyboard::feed((char)Keyboard::KEY_RETURN, 1);
            Keyboard::feed((char)Keyboard::KEY_RETURN, 0);
        } else {
            unichar c = [txt.text characterAtIndex:(txt.text.length - 1)];
            [submittedText appendFormat:@"%C", c];
            Keyboard::feedText(c);
        }
        textField.text = lastString = @"AAAAAAAAAAAAAAAAAAAA";
    }
}
- (BOOL)textFieldShouldReturn:(UITextField *)theTextField {
    Keyboard::feed((char)Keyboard::KEY_RETURN, 1);
    Keyboard::feed((char)Keyboard::KEY_RETURN, 0);
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSString stringWithString:submittedText] forKey:MCPEKeyboardSubmittedTextKey];
    [[NSNotificationCenter defaultCenter] postNotificationName:MCPEKeyboardSubmittedNotification object:self userInfo:userInfo];
    return NO;
}
@end
