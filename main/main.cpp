#include "whale.h"


class Sandbox : public Whale::Application {
public:
	Sandbox() {}
	~Sandbox() {}
};

Whale::Application* Whale::createApplication() {
	return new Sandbox();
}