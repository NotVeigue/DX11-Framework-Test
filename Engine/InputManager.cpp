#include "InputManager.h"
//#include <iostream>

// ----------------------------------------------------------------------------------
// Singleton Stuff
// ----------------------------------------------------------------------------------
template<> InputManager* Singleton<InputManager>::msSingleton = 0;
InputManager& InputManager::GetSingleton(void)
{
	assert(msSingleton);
	return *msSingleton;
}

InputManager* InputManager::GetSingletonPtr(void)
{
	return msSingleton;
}

// ----------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------
InputManager::InputManager()
{
	for (int i = 0; i < 256; i++)
	{
		m_heldKeys[i] = m_pressedKeys[i] = false;
		m_keydownCallbacks[i] = m_keyupCallbacks[i] = nullptr;
	}
}

InputManager::~InputManager()
{
}

void InputManager::SetKeyDown(unsigned int key)
{
	//std::printf("Key Down: %d, Hex: %X \n", key, key);
	m_pressedKeys[key] = true;

	// Keycode 0 = Any key
	m_pressedKeys[0] = true;

	TriggerInputCallbacks(INPUT_CALLBACK_TYPE::KEYDOWN, key);
	TriggerInputCallbacks(INPUT_CALLBACK_TYPE::KEYDOWN, 0, key);
}

void InputManager::SetKeyUp(unsigned int key)
{
	//std::printf("Key Up: %d, Hex: %X\n", key, key);
	m_heldKeys[key] = m_pressedKeys[key] = false;

	TriggerInputCallbacks(INPUT_CALLBACK_TYPE::KEYUP, key);
	TriggerInputCallbacks(INPUT_CALLBACK_TYPE::KEYUP, 0, key);

	// At this point, we need to check to see if any keys are held at all so we know whether or not
	// to clear the 'Any' flag for held keys.
	for (int i = 1; i < 256; i++)
	{
		if (m_heldKeys[i])
			return;
	}

	m_heldKeys[0] = false;
}

void InputManager::Update()
{
	// Iterate over the arrays of keys, clearing keys pressed this frame and marking them as held.
	for (int i = 0; i < 256; i++)
	{
		m_heldKeys[i] = m_pressedKeys[i] ? true : m_heldKeys[i];
		m_pressedKeys[i] = false;
	}
}

bool InputManager::IsKeyDown(unsigned int key) const
{
	assert(key < 256);
	return m_pressedKeys[key];
}

bool InputManager::IsKeyUp(unsigned int key) const
{
	assert(key < 256);
	return !m_pressedKeys[key] && !m_heldKeys[key];
}

bool InputManager::IsKeyHeld(unsigned int key) const
{
	assert(key < 256);
	return m_heldKeys[key];
}

void InputManager::TriggerInputCallbacks(INPUT_CALLBACK_TYPE callbackType, unsigned int key, unsigned int keyPressed)
{
	assert(key < 256);
	InputCallbackNode** callbackArray = 
		callbackType == INPUT_CALLBACK_TYPE::KEYDOWN ? 
		m_keydownCallbacks : 
		m_keyupCallbacks;

	InputCallbackNode* callbackNode = callbackArray[key];
	while(callbackNode != nullptr)
	{
		if (key == 0)
			reinterpret_cast<AnyInputCallback>(callbackNode->callbackFunc)(keyPressed);
		else
			reinterpret_cast<InputCallback>(callbackNode->callbackFunc)();

		callbackNode = callbackNode->next;
	}
}

void InputManager::RegisterInputCallback(void* callback, INPUT_CALLBACK_TYPE callbackType, unsigned int key)
{
	InputCallbackNode** callbackArray =
		callbackType == INPUT_CALLBACK_TYPE::KEYDOWN ?
		m_keydownCallbacks :
		m_keyupCallbacks;

	// Append the new callback node as the new head of the list
	InputCallbackNode* callbackListHead = callbackArray[key];
	InputCallbackNode* callbackNode = new InputCallbackNode();
	callbackNode->callbackFunc = callback;
	callbackNode->next = callbackListHead;
	callbackArray[key] = callbackNode;
}	

bool InputManager::RemoveInputCallback(void* callback, INPUT_CALLBACK_TYPE callbackType, unsigned int key)
{
	InputCallbackNode** callbackArray =
		callbackType == INPUT_CALLBACK_TYPE::KEYDOWN ?
		m_keydownCallbacks :
		m_keyupCallbacks;

	InputCallbackNode* callbackListHead = callbackArray[key];
	InputCallbackNode* callbackNode = callbackListHead;

	// Starting from the head node, iterate through the list in search of a node that holds the
	// specified callback function.
	InputCallbackNode* previousNode = nullptr;
	while (callbackNode != nullptr && callbackNode->callbackFunc != callback)
	{
		previousNode = callbackNode;
		callbackNode = callbackNode->next;
	}

	// If we reached the end of the list without finding a matching node, return false to
	// indicate that nothing was removed.
	if (callbackNode == nullptr)
		return false;

	// If the found node happens to be the head of the list, make sure its next node is made the new head.
	if (callbackNode == callbackListHead)
	{
		callbackArray[key] = callbackListHead->next;
	}
	else
	{
		previousNode->next = callbackNode->next;
	}

	delete callbackNode;
	return true;
}