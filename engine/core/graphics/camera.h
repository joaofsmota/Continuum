#ifndef CAMERA_H
#define CAMERA_H

#include <assert.h>
#include <algorithm>

#include "glm/gtx/euler_angles.hpp"

namespace Continuum {

	struct AngleProcUtils {
		static inline float clip_angle(const float d)
		{
			if (d < -180.0f) return d + 360.0f;
			if (d > +180.0f) return d - 360.f;
			return d;
		}
		static inline glm::vec3 clip_angles(const glm::vec3& angles)
		{
			return glm::vec3(
				std::fmod(angles.x, 360.0f),
				std::fmod(angles.y, 360.0f),
				std::fmod(angles.z, 360.0f)
			);
		}
		static inline glm::vec3 angle_delta(const glm::vec3& angles_current, const glm::vec3& angles_desired)
		{
			const glm::vec3 d = clip_angles(angles_current) - clip_angles(angles_desired);
			return glm::vec3(clip_angle(d.x), clip_angle(d.y), clip_angle(d.z));
		}
	};

	struct Camera {

		struct CameraPositionerInterface
		{
			virtual ~CameraPositionerInterface() = default;
		public:
			virtual glm::mat4 get_view_matrix() const = 0;
			virtual glm::vec3 get_position() const = 0;
		};

		struct camera_t final
		{
			explicit camera_t(CameraPositionerInterface& positioner)
			: positioner_(&positioner)
		{}
			camera_t(const camera_t&) = default;
			camera_t& operator = (const camera_t&) = default;
		public:
			inline glm::mat4 get_view_matrix() const { return positioner_->get_view_matrix(); }
			inline glm::vec3 get_position() const { return positioner_->get_position(); }
		private:
			const CameraPositionerInterface* positioner_; 
		};

		struct OrbCameraPositioner final : public CameraPositionerInterface
		{
			OrbCameraPositioner() = default;
			OrbCameraPositioner(const glm::vec3& camera_pos, const glm::vec3& target, const glm::vec3& up)
				: camera_position_(camera_pos)
				, camera_orientation_(glm::lookAt(camera_pos, target, up))
				, up_(up)
			{}
		public:
			void update(const double delta_sec, const glm::vec2& mouse_pos, const bool mouse_pressed) {

				if (mouse_pressed)
				{
					const glm::vec2 delta = mouse_pos - this->mouse_position_;
					const glm::quat delta_quat = glm::quat(glm::vec3(this->mouse_speed_ * delta.y, this->mouse_speed_ * delta.x, 0.0f));
					this->camera_orientation_ = delta_quat * this->camera_orientation_;
					this->camera_orientation_ = glm::normalize(this->camera_orientation_);
					set_up_vector(this->up_);
				}
				this->mouse_position_ = mouse_pos;

				const glm::mat4 v = glm::mat4_cast(this->camera_orientation_);

				const glm::vec3 forward = -glm::vec3(v[0][2], v[1][2], v[2][2]);
				const glm::vec3 right = glm::vec3(v[0][0], v[1][0], v[2][0]);
				const glm::vec3 up = glm::cross(right, forward);

				glm::vec3 accel(0.0f);

				if (MOVEMENT_.forward_) accel += forward;
				if (MOVEMENT_.backward_) accel -= forward;

				if (MOVEMENT_.left_) accel -= right;
				if (MOVEMENT_.right_) accel += right;

				if (MOVEMENT_.up_) accel += up;
				if (MOVEMENT_.down_) accel -= up;

				if (MOVEMENT_.fast_speed_) accel *= this->fast_coef_;

				if (accel == glm::vec3(0))
				{
					// decelerate naturally according to the damping value
					this->move_speed_ -= this->move_speed_ * std::min((1.0f / damping_) * static_cast<float>(delta_sec), 1.0f);
				}
				else
				{
					// acceleration
					this->move_speed_ += accel * acceleration_ * static_cast<float>(delta_sec);
					const float max_speed = MOVEMENT_.fast_speed_ ? this->max_speed_ * this->fast_coef_ : this->max_speed_;
					if (glm::length(this->move_speed_) > max_speed) this->move_speed_ = glm::normalize(this->move_speed_) * max_speed;
				}

				this->camera_position_ += this->move_speed_ * static_cast<float>(delta_sec);
			}
		public:
			virtual glm::mat4 get_view_matrix() const override
			{
				const glm::mat4 t = glm::translate(glm::mat4(1.0f), -this->camera_position_);
				const glm::mat4 r = glm::mat4_cast(this->camera_orientation_);
				return r * t;
			}
			virtual glm::vec3 get_position() const override
			{
				return this->camera_position_;
			}
		public:
			void set_position(const glm::vec3& camera_pos) { this->camera_position_ = camera_pos; }
			void reset_mouse_position(const glm::vec2& mouse_pos) { this->mouse_position_ = mouse_pos; };
			void set_up_vector(const glm::vec3& up)
			{
				const glm::mat4 view = get_view_matrix();
				const glm::vec3 dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
				this->camera_orientation_ = glm::lookAt(this->camera_position_, this->camera_position_ + dir, up);
			}
			inline void look_at(const glm::vec3& camera_pos, const glm::vec3& target, const glm::vec3& up) {
				this->camera_position_ = camera_pos;
				this->camera_orientation_ = glm::lookAt(camera_pos, target, up);
			}
		public:
			struct movement_t
			{
				bool forward_ = false;
				bool backward_ = false;
				bool left_ = false;
				bool right_ = false;
				bool up_ = false;
				bool down_ = false;
				//
				bool fast_speed_ = false;
			} MOVEMENT_;
		public:
			float mouse_speed_ = 4.0f;
			float acceleration_ = 150.0f;
			float damping_ = 0.2f;
			float max_speed_ = 10.0f;
			float fast_coef_ = 10.0f;
		private:
			glm::vec2 mouse_position_ = glm::vec2(0);
			glm::vec3 camera_position_ = glm::vec3(0.0f, 10.0f, 10.0f);
			glm::quat camera_orientation_ = glm::quat(glm::vec3(0));
			glm::vec3 move_speed_ = glm::vec3(0.0f);
			glm::vec3 up_ = glm::vec3(0.0f, 0.0f, 1.0f);
		};

		struct UICameraPositioner final : public CameraPositionerInterface
		{
			UICameraPositioner(const glm::vec3& camera_pos, const glm::vec3& angles)
				: position_current_(camera_pos)
				, position_desired_(camera_pos)
				, angles_current_(angles)
				, angles_desired_(angles)
			{}
		public:
			void update(const float delta_sec, const glm::vec2& mouse_pos, const bool mouse_pressed)
			{
				this->position_current_ += this->damping_linear_ * delta_sec * (this->position_desired_ - this->position_current_);

				// normalization is required to avoid "spinning" around the object 2pi times
				this->angles_current_ = AngleProcUtils::clip_angles(this->angles_current_);
				this->angles_desired_ = AngleProcUtils::clip_angles(this->angles_desired_);

				// update angles
				this->angles_current_ -= AngleProcUtils::angle_delta(this->angles_current_, this->angles_desired_) * this->damping_euler_angles_ * delta_sec;

				// normalize new angles
				this->angles_current_ = AngleProcUtils::clip_angles(this->angles_current_);

				const glm::vec3 a = glm::radians(this->angles_current_);

				this->current_transform_ = glm::translate(glm::yawPitchRoll(a.y, a.x, a.z), -this->position_current_);
			}
		public:
			inline void set_position(const glm::vec3& camera_pos) { this->position_current_ = camera_pos; }
			inline void set_angles(const float pitch, const float pan, const float roll) { this->angles_current_ = glm::vec3(pitch, pan, roll); }
			inline void set_angles(const glm::vec3& angles) { this->angles_current_ = angles; }
			inline void set_desired_position(const glm::vec3& camera_pos) { this->position_desired_ = camera_pos; }
			inline void set_desired_angles(const float pitch, const float pan, const float roll) { this->angles_desired_ = glm::vec3(pitch, pan, roll); }
			inline void set_desired_angles(const glm::vec3& angles) { this->angles_desired_ = angles; }
		public:
			virtual glm::vec3 get_position() const override { return this->position_current_; }
			virtual glm::mat4 get_view_matrix() const override { return this->current_transform_; }
		public:
			float damping_linear_ = 10.0f;
			glm::vec3 damping_euler_angles_ = glm::vec3(5.0f, 5.0f, 5.0f);
		private:
			glm::vec3 position_current_ = glm::vec3(0.0f);
			glm::vec3 position_desired_ = glm::vec3(0.0f);
			/// pitch, pan, roll
			glm::vec3 angles_current_ = glm::vec3(0.0f);
			glm::vec3 angles_desired_ = glm::vec3(0.0f);
			glm::mat4 current_transform_ = glm::mat4(1.0f);
		};

	};

}
#endif 