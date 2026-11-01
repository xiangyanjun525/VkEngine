#pragma once
#include "Base.h"

namespace Engine
{
	class Camera
	{
	public:
		using Ptr = std::shared_ptr<Camera>;
		static Ptr create()
		{
			return std::make_shared<Camera>();
		}

		enum CameraType { lookat, firstperson };
		CameraType mType = CameraType::lookat;

		glm::vec3 mRotation = glm::vec3();
		glm::vec3 mPosition = glm::vec3();
		glm::vec4 mViewPos = glm::vec4();

		float mRotationSpeed = 1.0f;
		float mMovementSpeed = 1.0f;

		bool mUpdated = true;
		bool mFlipY = false;

		struct
		{
			glm::mat4 perspective;
			glm::mat4 view;
		} mMatrices;

		struct
		{
			bool left = false;
			bool right = false;
			bool up = false;
			bool down = false;
		} mKeys;

		bool moving() const
		{
			return mKeys.left || mKeys.right || mKeys.up || mKeys.down;
		}

		float getNearClip() const {
			return mZnear;
		}

		float getFarClip() const {
			return mZfar;
		}

		void setPerspective(float fov, float aspect, float znear, float zfar)
		{
			glm::mat4 currentMatrix = mMatrices.perspective;
			this->mFov = fov;
			this->mZnear = znear;
			this->mZfar = zfar;
			mMatrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
			if (mFlipY) {
				mMatrices.perspective[1][1] *= -1.0f;
			}
			if (mMatrices.view != currentMatrix) {
				mUpdated = true;
			}
		};

		void updateAspectRatio(float aspect)
		{
			glm::mat4 currentMatrix = mMatrices.perspective;
			mMatrices.perspective = glm::perspective(glm::radians(mFov), aspect, mZnear, mZfar);
			if (mFlipY) {
				mMatrices.perspective[1][1] *= -1.0f;
			}
			if (mMatrices.view != currentMatrix) {
				mUpdated = true;
			}
		}

		void setPosition(glm::vec3 position)
		{
			this->mPosition = position;
			updateViewMatrix();
		}

		void setRotation(glm::vec3 rotation)
		{
			this->mRotation = rotation;
			updateViewMatrix();
		}

		void rotate(glm::vec3 delta)
		{
			this->mRotation += delta;
			updateViewMatrix();
		}

		void setTranslation(glm::vec3 translation)
		{
			this->mPosition = translation;
			updateViewMatrix();
		};

		void translate(glm::vec3 delta)
		{
			this->mPosition += delta;
			updateViewMatrix();
		}

		void setRotationSpeed(float rotationSpeed)
		{
			this->mRotationSpeed = rotationSpeed;
		}

		void setMovementSpeed(float movementSpeed)
		{
			this->mMovementSpeed = movementSpeed;
		}

		void update(float deltaTime)
		{
			mUpdated = false;
			if (mType == CameraType::firstperson)
			{
				if (moving())
				{
					glm::vec3 camFront;
					camFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
					camFront.y = sin(glm::radians(mRotation.x));
					camFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));
					camFront = glm::normalize(camFront);

					float moveSpeed = deltaTime * mMovementSpeed;

					if (mKeys.up)
						mPosition += camFront * moveSpeed;
					if (mKeys.down)
						mPosition -= camFront * moveSpeed;
					if (mKeys.left)
						mPosition -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
					if (mKeys.right)
						mPosition += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				}
			}
			updateViewMatrix();
		};

		// Update camera passing separate axis data (gamepad)
		// Returns true if view or position has been changed
		bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
		{
			bool retVal = false;

			if (mType == CameraType::firstperson)
			{
				// Use the common console thumbstick layout		
				// Left = view, right = move

				const float deadZone = 0.0015f;
				const float range = 1.0f - deadZone;

				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
				camFront.y = sin(glm::radians(mRotation.x));
				camFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));
				camFront = glm::normalize(camFront);

				float moveSpeed = deltaTime * mMovementSpeed * 2.0f;
				float rotSpeed = deltaTime * mRotationSpeed * 50.0f;

				// Move
				if (fabsf(axisLeft.y) > deadZone)
				{
					float pos = (fabsf(axisLeft.y) - deadZone) / range;
					mPosition -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
					retVal = true;
				}
				if (fabsf(axisLeft.x) > deadZone)
				{
					float pos = (fabsf(axisLeft.x) - deadZone) / range;
					mPosition += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
					retVal = true;
				}

				// Rotate
				if (fabsf(axisRight.x) > deadZone)
				{
					float pos = (fabsf(axisRight.x) - deadZone) / range;
					mRotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
					retVal = true;
				}
				if (fabsf(axisRight.y) > deadZone)
				{
					float pos = (fabsf(axisRight.y) - deadZone) / range;
					mRotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
					retVal = true;
				}
			}
			else
			{
				// todo: move code from example base class for look-at
			}

			if (retVal)
			{
				updateViewMatrix();
			}

			return retVal;
		}
	private:
		float mFov;
		float mZnear, mZfar;

		void updateViewMatrix()
		{
			glm::mat4 currentMatrix = mMatrices.view;

			glm::mat4 rotM = glm::mat4(1.0f);
			glm::mat4 transM;

			rotM = glm::rotate(rotM, glm::radians(mRotation.x * (mFlipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::vec3 translation = mPosition;
			if (mFlipY) {
				translation.y *= -1.0f;
			}
			transM = glm::translate(glm::mat4(1.0f), translation);

			if (mType == CameraType::firstperson)
			{
				mMatrices.view = rotM * transM;
			}
			else
			{
				mMatrices.view = transM * rotM;
			}

			mViewPos = glm::vec4(mPosition, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

			if (mMatrices.view != currentMatrix) {
				mUpdated = true;
			}
		}
	};
}

