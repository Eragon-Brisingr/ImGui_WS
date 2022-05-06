// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "imgui.h"

/*
KEY NAME	                EVENT.WHICH	        EVENT.KEY	        EVENT.CODE	        NOTES
backspace	                8	                Backspace	        Backspace	
tab	                        9	                Tab	                Tab	    
enter	                    13	                Enter	            Enter	
shift(left)	                16	                Shift	            ShiftLeft	        event.shiftKey is true
shift(right)	            16	                Shift	            ShiftRight	        event.shiftKey is true
ctrl(left)	                17	                Control	            ControlLeft	        event.ctrlKey is true
ctrl(right)	                17	                Control	            ControlRight	    event.ctrlKey is true
alt(left)	                18	                Alt	                AltLeft	            event.altKey is true
alt(right)	                18	                Alt	                AltRight	        event.altKey is true
pause/break	                19	                Pause	            Pause	
caps lock	                20	                CapsLock	        CapsLock	
escape	                    27	                Escape	            Escape	
space	                    32	                                    Space	            The event.key value is a single space.
page up	                    33	                PageUp	            PageUp	
page down	                34	                PageDown	        PageDown	
end	                        35	                End	                End	
home	                    36	                Home	            Home	
left arrow	                37	                ArrowLeft	        ArrowLeft	
up arrow	                38	                ArrowUp	            ArrowUp	
right arrow	                39	                ArrowRight	        ArrowRight	
down arrow	                40	                ArrowDown	        ArrowDown	
print screen	            44	                PrintScreen	        PrintScreen	
insert	                    45	                Insert	            Insert	
delete	                    46	                Delete	            Delete	
0	                        48	                0	                Digit0	
1	                        49	                1	                Digit1	
2	                        50	                2	                Digit2	
3	                        51	                3	                Digit3	
4	                        52	                4	                Digit4	
5	                        53	                5	                Digit5	
6	                        54	                6	                Digit6	
7	                        55	                7	                Digit7	
8	                        56	                8	                Digit8	
9	                        57	                9	                Digit9	
a	                        65	                a	                KeyA	
b	                        66	                b	                KeyB	
c	                        67	                c	                KeyC	
d	                        68	                d	                KeyD	
e	                        69	                e	                KeyE	
f	                        70	                f	                KeyF	
g	                        71	                g	                KeyG	
h	                        72	                h	                KeyH	
i	                        73	                i	                KeyI	
j	                        74	                j	                KeyJ	
k	                        75	                k	                KeyK	
l	                        76	                l	                KeyL	
m	                        77	                m	                KeyM	
n	                        78	                n	                KeyN	
o	                        79	                o	                KeyO	
p	                        80	                p	                KeyP	
q	                        81	                q	                KeyQ	
r	                        82	                r	                KeyR	
s	                        83	                s	                KeyS	
t	                        84	                t	                KeyT	
u	                        85	                u	                KeyU	
v	                        86	                v	                KeyV	
w	                        87	                w	                KeyW	
x	                        88	                x	                KeyX	
y	                        89	                y	                KeyY	
z	                        90	                z	                KeyZ	
left window key	            91	                Meta	            MetaLeft	        event.metaKey is true
right window key	        92	                Meta	            MetaRight	        event.metaKey is true
select key (Context Menu)	93	                ContextMenu	        ContextMenu	
numpad 0	                96	                0	                Numpad0	
numpad 1	                97	                1	                Numpad1	
numpad 2	                98	                2	                Numpad2	
numpad 3	                99	                3	                Numpad3	
numpad 4	                100	                4	                Numpad4	
numpad 5	                101	                5	                Numpad5	
numpad 6	                102	                6	                Numpad6	
numpad 7	                103	                7	                Numpad7	
numpad 8	                104	                8	                Numpad8	
numpad 9	                105	                9	                Numpad9	
multiply	                106	                *	                NumpadMultiply	
add	                        107	                +	                NumpadAdd	
subtract	                109	                -	                NumpadSubtract	
decimal point	            110	                .	                NumpadDecimal	
divide	                    111	                /	                NumpadDivide	
f1	                        112	                F1	                F1	
f2	                        113	                F2	                F2	
f3	                        114	                F3	                F3	
f4	                        115	                F4	                F4	
f5	                        116	                F5	                F5	
f6	                        117	                F6	                F6	
f7	                        118	                F7	                F7	
f8	                        119	                F8	                F8	
f9	                        120	                F9	                F9	
f10	                        121	                F10	                F10	
f11	                        122	                F11	                F11	
f12	                        123	                F12	                F12	
num lock	                144	                NumLock	            NumLock	
scroll lock	                145	                ScrollLock	        ScrollLock	
audio volume mute	        173	                AudioVolumeMute		                    ⚠️ The event.which value is 181 in Firefox. Also FF provides the code value as, VolumeMute
audio volume down	        174	                AudioVolumeDown		                    ⚠️ The event.which value is 182 in Firefox. Also FF provides the code value as, VolumeDown
audio volume up	            175	                AudioVolumeUp		                    ⚠️ The event.which value is 183 in Firefox. Also FF provides the code value as, VolumeUp
media player	            181	                LaunchMediaPlayer	                    ⚠️ The ️event.which value is 0(no value) in Firefox. Also FF provides the code value as, MediaSelect
launch application 1	    182	                LaunchApplication1	                    ⚠️ The ️event.which value is 0(no value) in Firefox. Also FF provides the code value as, LaunchApp1
launch application 2	    183	                LaunchApplication2	                    ⚠️ The ️event.which value is 0(no value) in Firefox. Also FF provides the code value as, LaunchApp2
semi-colon	                186	                ;	                Semicolon	        ⚠️ The event.which value is 59 in Firefox
equal sign	                187	                =	                Equal	            ⚠️ The event.which value is 61 in Firefox
comma	                    188	                ,	                Comma	        
dash	                    189	                -	                Minus	            ⚠️ The event.which value is 173 in Firefox
period	                    190	                .	                Period	
forward slash	            191	                /	                Slash	
Backquote/Grave accent	    192	                `	                Backquote	
open bracket	            219	                [	                BracketLeft	
back slash	                220	                \	                Backslash	
close bracket	            221	                ]	                BracketRight	
single quote	            222	                '	                Quote
*/

enum class EWebKeyCode : uint32
{
	Backspace						= 8,
	Tab	    						= 9,
	Enter							= 13,
	ShiftLeft	  					= 16,
	ShiftRight	  					= 16,
	ControlLeft	  					= 17,
	ControlRight					= 17,
	AltLeft	      					= 18,
	AltRight	  					= 18,
	Pause							= 19,
	CapsLock						= 20,
	Escape							= 27,
	Space	      					= 32,
	PageUp							= 33,
	PageDown						= 34,
	End								= 35,
	Home							= 36,
	ArrowLeft						= 37,
	ArrowUp							= 38,
	ArrowRight						= 39,
	ArrowDown						= 40,
	PrintScreen						= 44,
	Insert							= 45,
	Delete							= 46,
	Digit0							= 48,
	Digit1							= 49,
	Digit2							= 50,
	Digit3							= 51,
	Digit4							= 52,
	Digit5							= 53,
	Digit6							= 54,
	Digit7							= 55,
	Digit8							= 56,
	Digit9							= 57,
	KeyA							= 65,
	KeyB							= 66,
	KeyC							= 67,
	KeyD							= 68,
	KeyE							= 69,
	KeyF							= 70,
	KeyG							= 71,
	KeyH							= 72,
	KeyI							= 73,
	KeyJ							= 74,
	KeyK							= 75,
	KeyL							= 76,
	KeyM							= 77,
	KeyN							= 78,
	KeyO							= 79,
	KeyP							= 80,
	KeyQ							= 81,
	KeyR							= 82,
	KeyS							= 83,
	KeyT							= 84,
	KeyU							= 85,
	KeyV							= 86,
	KeyW							= 87,
	KeyX							= 88,
	KeyY							= 89,
	KeyZ							= 90,
	MetaLeft	  					= 91,
	MetaRight	  					= 92,
	ContextMenu						= 93,
	Numpad0							= 96,
	Numpad1							= 97,
	Numpad2							= 98,
	Numpad3							= 99,
	Numpad4							= 100,
	Numpad5							= 101,
	Numpad6							= 102,
	Numpad7							= 103,
	Numpad8							= 104,
	Numpad9							= 105,
	NumpadMultiply					= 106,
	NumpadAdd						= 107,
	NumpadSubtract					= 109,
	NumpadDecimal					= 110,
	NumpadDivide					= 111,
	F1								= 112,
	F2								= 113,
	F3								= 114,
	F4								= 115,
	F5								= 116,
	F6								= 117,
	F7								= 118,
	F8								= 119,
	F9								= 120,
	F10								= 121,
	F11								= 122,
	F12								= 123,
	NumLock							= 144,
	ScrollLock						= 145,
	AudioVolumeMute					= 173,
	AudioVolumeDown					= 174,
	AudioVolumeUp					= 175,
	LaunchMediaPlayer				= 181,
	LaunchApplication1				= 182,
	LaunchApplication2Semicolon	  	= 183,
	Equal	      					= 186,
	Comma	      					= 187,
	Minus	      					= 188,
	Period							= 189,
	Slash							= 190,
	Backquote						= 191,
	BracketLeft						= 192,
	Backslash						= 219,
	BracketRight					= 220,
	Quote							= 221,
};

inline ImGuiKey ToImGuiKey(EWebKeyCode WebKeyCode)
{
	switch (WebKeyCode)
	{
	case EWebKeyCode::Backspace:							return ImGuiKey_Backspace;
	case EWebKeyCode::Tab:									return ImGuiKey_Tab;
	case EWebKeyCode::Enter:								return ImGuiKey_Enter;
	case EWebKeyCode::ShiftLeft:							return ImGuiKey_LeftShift;
	// case EWebKeyCode::ShiftRight:						return ImGuiKey_RightShift;
	case EWebKeyCode::ControlLeft:							return ImGuiKey_LeftCtrl;
	// case EWebKeyCode::ControlRight: 						return ImGuiKey_RightCtrl;			
	case EWebKeyCode::AltLeft: 								return ImGuiKey_LeftAlt;	
	// case EWebKeyCode::AltRight: 							return ImGuiKey_RightAlt;		
	case EWebKeyCode::Pause: 								return ImGuiKey_Pause;	
	case EWebKeyCode::CapsLock: 							return ImGuiKey_CapsLock;		
	case EWebKeyCode::Escape: 								return ImGuiKey_Escape;	
	case EWebKeyCode::Space: 								return ImGuiKey_Space;	
	case EWebKeyCode::PageUp: 								return ImGuiKey_PageUp;	
	case EWebKeyCode::PageDown: 							return ImGuiKey_PageDown;		
	case EWebKeyCode::End: 									return ImGuiKey_End;
	case EWebKeyCode::Home: 								return ImGuiKey_Home;	
	case EWebKeyCode::ArrowLeft: 							return ImGuiKey_LeftArrow;		
	case EWebKeyCode::ArrowUp: 								return ImGuiKey_UpArrow;	
	case EWebKeyCode::ArrowRight: 							return ImGuiKey_RightArrow;		
	case EWebKeyCode::ArrowDown: 							return ImGuiKey_DownArrow;		
	case EWebKeyCode::PrintScreen: 							return ImGuiKey_PrintScreen;		
	case EWebKeyCode::Insert: 								return ImGuiKey_Insert;	
	case EWebKeyCode::Delete: 								return ImGuiKey_Delete;	
	case EWebKeyCode::Digit0: 								return ImGuiKey_0;	
	case EWebKeyCode::Digit1: 								return ImGuiKey_1;	
	case EWebKeyCode::Digit2: 								return ImGuiKey_2;	
	case EWebKeyCode::Digit3: 								return ImGuiKey_3;	
	case EWebKeyCode::Digit4: 								return ImGuiKey_4;	
	case EWebKeyCode::Digit5: 								return ImGuiKey_5;	
	case EWebKeyCode::Digit6: 								return ImGuiKey_6;	
	case EWebKeyCode::Digit7: 								return ImGuiKey_7;	
	case EWebKeyCode::Digit8: 								return ImGuiKey_8;	
	case EWebKeyCode::Digit9: 								return ImGuiKey_9;	
	case EWebKeyCode::KeyA: 								return ImGuiKey_A;	
	case EWebKeyCode::KeyB: 								return ImGuiKey_B;	
	case EWebKeyCode::KeyC: 								return ImGuiKey_C;	
	case EWebKeyCode::KeyD: 								return ImGuiKey_D;	
	case EWebKeyCode::KeyE: 								return ImGuiKey_E;	
	case EWebKeyCode::KeyF: 								return ImGuiKey_F;	
	case EWebKeyCode::KeyG: 								return ImGuiKey_G;	
	case EWebKeyCode::KeyH: 								return ImGuiKey_H;	
	case EWebKeyCode::KeyI: 								return ImGuiKey_I;	
	case EWebKeyCode::KeyJ: 								return ImGuiKey_J;	
	case EWebKeyCode::KeyK: 								return ImGuiKey_K;	
	case EWebKeyCode::KeyL: 								return ImGuiKey_L;	
	case EWebKeyCode::KeyM: 								return ImGuiKey_M;	
	case EWebKeyCode::KeyN: 								return ImGuiKey_N;	
	case EWebKeyCode::KeyO: 								return ImGuiKey_O;	
	case EWebKeyCode::KeyP: 								return ImGuiKey_P;	
	case EWebKeyCode::KeyQ: 								return ImGuiKey_Q;	
	case EWebKeyCode::KeyR: 								return ImGuiKey_R;	
	case EWebKeyCode::KeyS: 								return ImGuiKey_S;	
	case EWebKeyCode::KeyT: 								return ImGuiKey_T;	
	case EWebKeyCode::KeyU: 								return ImGuiKey_U;	
	case EWebKeyCode::KeyV: 								return ImGuiKey_V;
	case EWebKeyCode::KeyW: 								return ImGuiKey_W;
	case EWebKeyCode::KeyX: 								return ImGuiKey_X;		
	case EWebKeyCode::KeyY: 								return ImGuiKey_Y;		
	case EWebKeyCode::KeyZ: 								return ImGuiKey_Z;		
	case EWebKeyCode::MetaLeft: 							return ImGuiKey_None;			
	case EWebKeyCode::MetaRight: 							return ImGuiKey_None;			
	case EWebKeyCode::ContextMenu: 							return ImGuiKey_None;			
	case EWebKeyCode::Numpad0: 								return ImGuiKey_Keypad0;
	case EWebKeyCode::Numpad1: 								return ImGuiKey_Keypad1;
	case EWebKeyCode::Numpad2: 								return ImGuiKey_Keypad2;
	case EWebKeyCode::Numpad3: 								return ImGuiKey_Keypad3;
	case EWebKeyCode::Numpad4: 								return ImGuiKey_Keypad4;
	case EWebKeyCode::Numpad5: 								return ImGuiKey_Keypad5;
	case EWebKeyCode::Numpad6: 								return ImGuiKey_Keypad6;
	case EWebKeyCode::Numpad7: 								return ImGuiKey_Keypad7;
	case EWebKeyCode::Numpad8: 								return ImGuiKey_Keypad8;
	case EWebKeyCode::Numpad9: 								return ImGuiKey_Keypad9;
	case EWebKeyCode::NumpadMultiply: 						return ImGuiKey_KeypadMultiply;				
	case EWebKeyCode::NumpadAdd: 							return ImGuiKey_KeypadSubtract;			
	case EWebKeyCode::NumpadSubtract: 						return ImGuiKey_KeypadSubtract;				
	case EWebKeyCode::NumpadDecimal: 						return ImGuiKey_KeypadDecimal;				
	case EWebKeyCode::NumpadDivide: 						return ImGuiKey_KeypadDivide;				
	case EWebKeyCode::F1: 									return ImGuiKey_F1;	
	case EWebKeyCode::F2: 									return ImGuiKey_F2;	
	case EWebKeyCode::F3: 									return ImGuiKey_F3;	
	case EWebKeyCode::F4: 									return ImGuiKey_F4;	
	case EWebKeyCode::F5: 									return ImGuiKey_F5;	
	case EWebKeyCode::F6: 									return ImGuiKey_F6;	
	case EWebKeyCode::F7: 									return ImGuiKey_F7;	
	case EWebKeyCode::F8: 									return ImGuiKey_F8;	
	case EWebKeyCode::F9: 									return ImGuiKey_F9;	
	case EWebKeyCode::F10: 									return ImGuiKey_F10;	
	case EWebKeyCode::F11: 									return ImGuiKey_F11;	
	case EWebKeyCode::F12: 									return ImGuiKey_F12;	
	case EWebKeyCode::NumLock: 								return ImGuiKey_NumLock;		
	case EWebKeyCode::ScrollLock: 							return ImGuiKey_ScrollLock;			
	case EWebKeyCode::AudioVolumeMute: 						return ImGuiKey_None;				
	case EWebKeyCode::AudioVolumeDown: 						return ImGuiKey_None;				
	case EWebKeyCode::AudioVolumeUp: 						return ImGuiKey_None;				
	case EWebKeyCode::LaunchMediaPlayer: 					return ImGuiKey_None;					
	case EWebKeyCode::LaunchApplication1: 					return ImGuiKey_None;					
	case EWebKeyCode::LaunchApplication2Semicolon: 			return ImGuiKey_None;							
	case EWebKeyCode::Equal: 								return ImGuiKey_Equal;		
	case EWebKeyCode::Comma: 								return ImGuiKey_Comma;		
	case EWebKeyCode::Minus: 								return ImGuiKey_Minus;		
	case EWebKeyCode::Period: 								return ImGuiKey_Period;		
	case EWebKeyCode::Slash: 								return ImGuiKey_Slash;		
	case EWebKeyCode::Backquote: 							return ImGuiKey_Apostrophe;			
	case EWebKeyCode::BracketLeft: 							return ImGuiKey_LeftBracket;			
	case EWebKeyCode::Backslash: 							return ImGuiKey_Backslash;			
	case EWebKeyCode::BracketRight: 						return ImGuiKey_RightBracket;				
	case EWebKeyCode::Quote: 								return ImGuiKey_Semicolon;		
	default: ;
	}
	return ImGuiKey_None;
}
