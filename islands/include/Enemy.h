#include "Component.h"
#include "PhysicalBody.h"
#include "Health.h"
#include "Effect.h"
#include "Sound.h"
#include "StateMachine.h"

namespace islands {
namespace enemy {

template <class T>
class Dead : public StateMachine<T>::State {
	using State::State;

	void start(T& parent) override {
		auto& entity = parent.getEntity();
		if (entity.hasComponent<Collider>()) {
			entity.getFirstComponent<Collider>()->clearCallbacks();
		}
		if (entity.hasComponent<DamageEffect>()) {
			entity.getFirstComponent<DamageEffect>()->destroy();
		}
		entity.createComponent<ScatterEffect>([&entity] {
			entity.destroy();
		});
		ResourceSystem::getInstance().get<Sound>("EnemyDieSound")->createInstance()->play();
	}
};

class Slime : public Component {
public:
	Slime() = default;
	virtual ~Slime() = default;

	void start() override;
	void update() override;

private:
	StateMachine<Slime> machine_;

	using State = StateMachine<Slime>::State;

	class Moving : public State {
		using State::State;
		void update(Slime& parent) override;
	};

	class Turning : public State {
		using State::State;
		void start(Slime& parent) override;
		void update(Slime& parent) override;

		glm::quat targetQuat_;
		glm::vec3 initPos_;
	};

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	glm::vec3 direction_;
};

class BigSlime : public Component {
public:
	BigSlime() = default;
	virtual ~BigSlime() = default;

	void start() override;
	void update() override;

private:
	StateMachine<BigSlime> machine_;

	using State = StateMachine<BigSlime>::State;

	class Pausing : public State {
		using State::State;
		void update(BigSlime& parent) override;
	};

	class Jumping : public State {
		using State::State;
		void start(BigSlime& parent) override;
		void update(BigSlime& parent) override;
	};

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
};

class Rabbit : public Component {
public:
	Rabbit() = default;
	virtual ~Rabbit() = default;

	void start() override;
	void update() override;

private:
	StateMachine<Rabbit> machine_;

	using State = StateMachine<Rabbit>::State;

	static constexpr auto JUMP_ANIM_START_TIME = 190;

	class Jumping : public State {
		using State::State;
		void start(Rabbit& parent) override;
		void update(Rabbit& parent) override;
	};

	class Pausing : public State {
		using State::State;
		void start(Rabbit& parent) override;
		void update(Rabbit& parent) override;
	};

	class PreAttack : public State {
		using State::State;
		void start(Rabbit& parent) override;
		void update(Rabbit& parent) override;
	};

	class Attacking : public State {
		using State::State;
		void start(Rabbit& parent) override;
		void update(Rabbit& parent) override;

		std::shared_ptr<Entity> attackEntity_;
	};

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	glm::vec3 direction_;
};

class Crab : public Component {
public:
	Crab() = default;
	virtual ~Crab() = default;

	void start() override;
	void update() override;

private:
	StateMachine<Crab> machine_;

	using State = StateMachine<Crab>::State;

	static const auto ATTACK_ANIM_START = 50;

	class Pausing : public State {
		using State::State;
		void start(Crab& parent) override;
		void update(Crab& parent) override;
	};

	class Moving : public State {
		using State::State;
		void start(Crab& parent) override;
		void update(Crab& parent) override;
	};

	class PreAttack : public State {
		using State::State;
		void start(Crab& parent) override;
		void update(Crab& parent) override;
	};

	class Attacking : public State {
		using State::State;
		void start(Crab& parent) override;
		void update(Crab& parent) override;

		std::shared_ptr<Entity> attackEntity_;
	};

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	glm::vec3 direction_;
};

class Dragon : public Component {
public:
	Dragon();
	virtual ~Dragon() = default;

	void start() override;
	void update() override;

private:
	StateMachine<Dragon> machine_;

	using State = StateMachine<Dragon>::State;

	class Hovering : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;
	};

	class Moving : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;

		double targetDelta_;
	};

	class PreFire : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;
	};

	class PostFire : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;
	};

	class Descending : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;

		double targetDelta_;
	};

	class Tackling : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;

		double targetDelta_;
	};

	class Ascending : public State {
		using State::State;
		void start(Dragon& parent) override;
		void update(Dragon& parent) override;

		double targetDelta_;
	};

	class Dead : public State {
		using State::State;
		void start(Dragon& parent) override;
	};

	std::mt19937 engine_;
	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	glm::vec3 direction_;

	void lookAtPlayer();
	void loopHoveringAnimation();

};

class TotemPoll : public Component {
public:
	TotemPoll();
	virtual ~TotemPoll() = default;

	void start() override;
	void update() override;

private:
	std::shared_ptr<Entity> playerEntity_;
	bool activated_;
};

}
}
