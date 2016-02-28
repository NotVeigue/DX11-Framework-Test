#pragma once
#include "Singleton.h"

// A helpful enum of some of the most common keycodes
// NOTE: I could have made this an enum class, but in this case, the
// auto type conversion of traditional enums is actually desireable 
// because I am using the enum values as indices into arrays.
enum KEY
{
	ANY		= 0x0,
	BACKSPC = 0x08,
	ENTER	= 0x0D,
	SHIFT	= 0x10,
	CTRL	= 0x11,
	ESC		= 0x1B,
	SPACE	= 0x20,
	LEFT	= 0x25,
	UP		= 0x26,
	RIGHT	= 0x27,
	DOWN	= 0x28,
	DEL		= 0x2E,
	K0		= 0x30,
	K1		= 0x31,
	K2		= 0x32,
	K3		= 0x33,
	K4		= 0x34,
	K5		= 0x35,
	K6		= 0x36,
	K7		= 0x37,
	K8		= 0x38,
	K9		= 0x39,
	A		= 0x41,
	B		= 0x42,
	C		= 0x43,
	D		= 0x44,
	E		= 0x45,
	F		= 0x46,
	G		= 0x47,
	H		= 0x48,
	I		= 0x49,
	J		= 0x4A,
	K		= 0x4B,
	L		= 0x4C,
	M		= 0x4D,
	N		= 0x4E,
	O		= 0x4F,
	P		= 0x50,
	Q		= 0x51,
	R		= 0x52,
	S		= 0x53,
	T		= 0x54,
	U		= 0x55,
	V		= 0x56,
	W		= 0x57,
	X		= 0x58,
	Y		= 0x59,
	Z		= 0x5A
};

enum class INPUT_CALLBACK_TYPE
{
	KEYDOWN,
	KEYUP
};

class InputManager : public Singleton<InputManager>
{
	// Allows SetKeyDown and SetKeyUp to be called from within WindowsManager
	friend class WindowsManager;

private:
	
	// Some typedefs to help with casting input callback functions before calling them
	typedef void(*InputCallback)();
	typedef void(*AnyInputCallback)(unsigned int);
	
	// These structs are used to form very rudimentary linked lists of callback functions.
	struct InputCallbackNode {
		void* callbackFunc;
		InputCallbackNode* next;
	};
	InputCallbackNode* m_keydownCallbacks[256];
	InputCallbackNode* m_keyupCallbacks[256];

	// Arrays for tracking the pressed/held state of every button tracked by the windows msg system.
	// For clarification, a button's state is only 'Pressed' on the initial frame it gets pressed on.
	// On subseqent frames, the state changes to 'Held' until a keyup message is received. I could have 
	// combined these into a single array, using 1 for pressed and 2 for held, but since I am using 0 to
	// store the state of 'any' key, this would preclude me from being able to see if 'any' key was pressed
	// on a given frame if 'any' key was already being held. Therefore, I decided to stick with two separate
	// arrays.
	bool m_heldKeys[256];
	bool m_pressedKeys[256];

	void SetKeyDown(unsigned int);
	void SetKeyUp(unsigned int);

	// A quick helper function to trigger the chain of callbacks associated with a specific key.
	// The second unsigned int represents the code of the actual key pressed. This is used in
	// cases where we want to trigger the 'Any' key callbacks and pass in the value of the actual key
	// pressed as the argument to the callback function.
	void TriggerInputCallbacks(INPUT_CALLBACK_TYPE, unsigned int, unsigned int = 0);
	
public:
	InputManager();
	~InputManager();

	static InputManager& GetSingleton(void);
	static InputManager* GetSingletonPtr(void);

	// Tells the manager that a frame has passed and that any keys that were pressed on the previous frame should be 
	// cleared and marked as held. This is my way of overcoming the short pause in windows messages between when a key
	// is first pressed and when additional messages with the keyheld flag begin to be sent.
	void Update();
	
	// Functions for querying the state of a given key
	bool IsKeyDown(unsigned int) const;
	bool IsKeyUp(unsigned int) const;
	bool IsKeyHeld(unsigned int) const;

	// Functions for adding and removing callback functions for a given key
	void RegisterInputCallback(void*, INPUT_CALLBACK_TYPE, unsigned int = 0);
	bool RemoveInputCallback(void*, INPUT_CALLBACK_TYPE, unsigned int = 0);
};

