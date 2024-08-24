#include <ros/ros.h>
#include <visualization_msgs/MarkerArray.h>

#include <people_msgs_utils/utils.h>

// global publisher for markers
ros::Publisher pub;

// individuals
visualization_msgs::MarkerArray createMarkers(
	const people_msgs_utils::People& people,
	const std::string& frame_id
) {
	visualization_msgs::MarkerArray marker_array;

	// default color with no transparency at all
	std_msgs::ColorRGBA color;
	color.a = 1.0;
	color.r = 1.0;
	color.g = 1.0;
	color.b = 1.0;

	// default header
	std_msgs::Header header;
	header.frame_id = frame_id;
	header.stamp = ros::Time::now();

	// default value of the marker's lifetime
	const ros::Duration MARKER_LIFETIME(1.0);

	// lambdas creating subsequent marker types
	auto create_shape_marker_fun = [=](
		const people_msgs_utils::Person& person,
		int person_id
	) -> visualization_msgs::Marker {
		visualization_msgs::Marker marker;
		marker.header = header;
		marker.ns = "individuals/shape";
		marker.type = visualization_msgs::Marker::CYLINDER;
		marker.action = visualization_msgs::Marker::ADD;
		marker.lifetime = MARKER_LIFETIME;
		marker.id = person_id;
		marker.pose = person.getPose();
		// scale of the shape
		marker.scale.x = 0.55;
		marker.scale.y = 0.55;
		marker.scale.z = 1.75;
		// adjust height (let the bottom be at the ground); must be executed once the scale is updated
		marker.pose.position.z += marker.scale.z / 2;

		marker.color = color;
		// will fade to a fully transparent marker when reliability becomes 0
		marker.color.a = person.getReliability();
		return marker;
	};

	auto create_text_marker_fun = [=](
		const people_msgs_utils::Person& person,
		int person_id,
		double shape_height,
		double shape_width
	) -> visualization_msgs::Marker {
		visualization_msgs::Marker marker;
		marker.header = header;
		marker.ns = "individuals/text";
		marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
		marker.action = visualization_msgs::Marker::ADD;
		marker.lifetime = MARKER_LIFETIME;
		marker.id = person_id;
		marker.pose = person.getPose();
		// adjust height (let the bottom of text be above the marker)
		marker.pose.position.z = 1.2 * shape_height;
		// scale of the text
		marker.scale.z = shape_width;

		marker.color = color;
		// will fade to a fully transparent marker when reliability becomes 0
		marker.color.a = person.getReliability();
		// let the marker show provided name of the person
		marker.text = person.getName();
		return marker;
	};

	auto create_orientation_marker_fun = [=](
		const people_msgs_utils::Person& person,
		int person_id,
		double shape_height,
		double shape_depth
	) -> visualization_msgs::Marker {
		visualization_msgs::Marker marker;
		marker.header = header;
		marker.ns = "individuals/orientation";
		marker.type = visualization_msgs::Marker::ARROW;
		marker.action = visualization_msgs::Marker::ADD;
		marker.lifetime = MARKER_LIFETIME;
		marker.id = person_id;
		marker.pose = person.getPose();
		// adjust height (let the bottom of the arrow be above the shape)
		marker.pose.position.z = shape_height;
		// scale of the arrow
		marker.scale.x = shape_depth;
		marker.scale.y = 0.1;
		marker.scale.z = 0.1;

		marker.color = color;
		// will fade to a fully transparent marker when reliability becomes 0
		marker.color.a = person.getReliability();
		return marker;
	};

	auto create_velocity_marker_fun = [=](
		const people_msgs_utils::Person& person,
		int person_id,
		double shape_depth
	) -> visualization_msgs::Marker {
		visualization_msgs::Marker marker;
		marker.header = header;
		marker.ns = "individuals/velocity";
		marker.type = visualization_msgs::Marker::ARROW;
		marker.action = visualization_msgs::Marker::ADD;
		marker.lifetime = MARKER_LIFETIME;
		marker.id = person_id;
		marker.pose.position = person.getPosition();
		// adjust height (let the bottom of the arrow be at the ground level)
		marker.pose.position.z = 0.0;

		// set marker's orientation according to the velocity of a person
		tf2::Quaternion vel_orientation;
		// NOTE: for a proper yaw angle, yaw, pitch and roll angles must be reordered (compared to the documentation)
		vel_orientation.setEuler(
			0.0,
			0.0,
			std::atan2(person.getVelocityY(), person.getVelocityX())
		);
		marker.pose.orientation = tf2::toMsg(vel_orientation);

		// scale of the arrow
		// velocity value based on "Moussaid et al., Experimental study (...), 2009"
		const double VEL_MAX = 1.29;
		double vel_magnitude = std::hypot(person.getVelocityX(), person.getVelocityY());
		double vel_percentage = std::min(vel_magnitude / VEL_MAX, 1.0);
		// shape_depth offset so the vel. vector is visible in front of the "shape"
		marker.scale.x = 0.5 * shape_depth + vel_percentage;
		marker.scale.y = 0.1;
		marker.scale.z = 0.1;

		marker.color = color;
		// will fade to a fully transparent marker when reliability becomes 0
		marker.color.a = person.getReliability();
		return marker;
	};

	for (const auto& person: people) {
		int person_id = 0;
		try {
			person_id = std::stoi(person.getName());
		} catch (const std::invalid_argument& e) {
			person_id = std::numeric_limits<int>::max();
			ROS_ERROR(
				"Could not convert '%s' to an integer! Assigned '%d' as a fallback person ID",
				person.getName().c_str(),
				person_id
			);
		}

		// create markers
		auto shape = create_shape_marker_fun(person, person_id);
		auto text = create_text_marker_fun(person, person_id, shape.scale.z, shape.scale.y);
		auto orientation = create_orientation_marker_fun(person, person_id, shape.scale.z, shape.scale.x);
		auto vel = create_velocity_marker_fun(person, person_id, shape.scale.x);

		// collect markers
		marker_array.markers.push_back(shape);
		marker_array.markers.push_back(text);
		marker_array.markers.push_back(orientation);
		marker_array.markers.push_back(vel);
	}

	return marker_array;
}

// groups
visualization_msgs::MarkerArray createMarkers(
	const people_msgs_utils::Groups& groups,
	const std::string& frame_id
) {
	visualization_msgs::MarkerArray marker_array;

	for (const auto& grp: groups) {
		visualization_msgs::Marker group_marker;
		group_marker.action = visualization_msgs::Marker::ADD;
		group_marker.ns = "groups/shape";
		group_marker.id = std::stoi(grp.getName());
		group_marker.color.a = grp.getReliability();
		group_marker.color.r = 0.1;
		group_marker.color.g = 0.5;
		group_marker.color.b = 0.4;
		group_marker.type = visualization_msgs::Marker::CYLINDER;
		group_marker.scale.x = grp.getSpanX();
		group_marker.scale.y = grp.getSpanY();
		group_marker.scale.z = 0.1;
		group_marker.pose = grp.getPose();
		group_marker.header.stamp = ros::Time::now();
		group_marker.header.frame_id = frame_id;
		group_marker.lifetime = ros::Duration(3.0);

		marker_array.markers.push_back(group_marker);

		for (const auto& member: grp.getMembers()) {
			visualization_msgs::Marker member_marker;
			member_marker.header = group_marker.header;
			member_marker.pose = member.getPose();
			member_marker.action = visualization_msgs::Marker::ADD;
			member_marker.type = visualization_msgs::Marker::CYLINDER;
			member_marker.ns = "groups/members";
			member_marker.id = std::stoi(member.getName());
			member_marker.color.a = member.getReliability();
			member_marker.color.r = 0.9;
			member_marker.color.g = 0.2;
			member_marker.color.b = 0.2;
			member_marker.lifetime = ros::Duration(1.0);
			member_marker.scale.x = 0.30;
			member_marker.scale.y = 0.30;
			member_marker.scale.z = 1.8;
			// apply offset so the marker is not below the ground
			member_marker.pose.position.z += member_marker.scale.z / 2.0;

			marker_array.markers.push_back(member_marker);
		}

		visualization_msgs::Marker cog_marker;
		cog_marker.action = visualization_msgs::Marker::ADD;
		cog_marker.ns = "groups/cog";
		cog_marker.id = std::stoi(grp.getName());
		cog_marker.color.a = grp.getReliability();
		cog_marker.color.r = 0.5;
		cog_marker.color.g = 0.1;
		cog_marker.color.b = 0.4;
		cog_marker.type = visualization_msgs::Marker::CYLINDER;
		cog_marker.scale.x = 0.35;
		cog_marker.scale.y = 0.35;
		cog_marker.scale.z = 0.10;
		cog_marker.pose.position.x = grp.getCenterOfGravity().x;
		cog_marker.pose.position.y = grp.getCenterOfGravity().y;
		// higher to make it visible
		cog_marker.pose.position.z = grp.getCenterOfGravity().z + 0.2;
		cog_marker.pose.orientation.w = 1.0;
		cog_marker.header.stamp = ros::Time::now();
		cog_marker.header.frame_id = frame_id;
		cog_marker.lifetime = ros::Duration(3.0);

		marker_array.markers.push_back(cog_marker);
	}

	return marker_array;
}

void peopleCallback(const people_msgs::People& people) {
	if (!pub.getNumSubscribers()) {
		// nothing to do
		return;
	}

	// convert to custom types
	people_msgs_utils::People ppl;
	people_msgs_utils::Groups grp;
	std::tie(ppl, grp) = people_msgs_utils::createFromPeople(people.people);
	// create markers
	auto markers_groups = createMarkers(grp, people.header.frame_id);
	auto markers_individuals = createMarkers(ppl, people.header.frame_id);

	// publish
	pub.publish(markers_groups);
	pub.publish(markers_individuals);
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "people_visualization");
	ros::NodeHandle nh;
	ros::Subscriber sub = nh.subscribe("/people", 5, peopleCallback);
	pub = nh.advertise<visualization_msgs::MarkerArray>("/people/markers", 5);
	ros::spin();
	return 0;
}
