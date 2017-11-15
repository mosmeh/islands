#pragma once

namespace islands {

template <typename T>
class StateMachine {
public:
	class State {
	public:
		State(StateMachine& machine) :
			machine_(machine),
			isFirstUpdate_(true),
			startedAt_(glfwGetTime()) {}

		void startAndUpdate(T& parent) {
			if (isFirstUpdate_) {
				start(parent);
				isFirstUpdate_ = false;
			}
			update(parent);
		}

	protected:
		virtual void start(T&) {}
		virtual void update(T&) {}

		double getElapsed() const {
			return glfwGetTime() - startedAt_;
		}

		template<class StateType, class... Args>
		std::enable_if_t<std::is_base_of<State, StateType>::value, void>
		changeState(Args&&... args) const {
			machine_.changeState<StateType, Args...>(std::forward(args)...);
		}

	private:
		StateMachine& machine_;
		bool isFirstUpdate_;
		double startedAt_;
	};

	StateMachine() = default;
	virtual ~StateMachine() = default;

	void update(T& parent) {
		if (current_) {
			current_->startAndUpdate(parent);
		}
	}

	template<class StateType, class... Args>
	std::enable_if_t<std::is_base_of<State, StateType>::value, void>
	changeState(Args&&... args) {
		current_ = std::make_shared<StateType>(*this, std::forward(args)...);
	}

private:
	std::shared_ptr<State> current_;
};

}
