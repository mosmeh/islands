#pragma once

namespace islands {

class VertexArray {
public:
	VertexArray() {
		glGenVertexArrays(1, &id_);
	}

	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;
	VertexArray(VertexArray&&) = default;

	virtual ~VertexArray() {
		glDeleteVertexArrays(1, &id_);
	}

	void bind() const {
		glBindVertexArray(id_);
	}

private:
	GLuint id_;
};

}
