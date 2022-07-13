#pragma once

class InputInterface {
public:
	InputInterface() = default;
	virtual ~InputInterface() = default;

	virtual bool OnRightTriggerPressed() { return false; }
	virtual bool OnRightTriggerReleased() { return false; }

	virtual bool OnLeftTriggerPressed() { return false; }
	virtual bool OnLeftTriggerReleased() { return false; }

	virtual bool OnLeftGripPressed() { return false; }
	virtual bool OnLeftGripReleased() { return false; }

	virtual bool OnLeftThumbstickY(const float Value) { return false; }
	virtual bool OnLeftThumbstickX(const float Value) { return false; }
	virtual bool OnLeftThumbstickLeft() { return false; }
	virtual bool OnLeftThumbstickRight() { return false; }
	virtual bool OnLeftThumbstickUp() { return false; }
	virtual bool OnLeftThumbstickDown() { return false; }
};
