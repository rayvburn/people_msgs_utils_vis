#include <ros/ros.h>
#include <visualization_msgs/MarkerArray.h>

#include <people_msgs_utils/utils.h>

// global publisher for markers
ros::Publisher pub;

visualization_msgs::MarkerArray createMarkers(
	const people_msgs_utils::Groups& groups,
	const std::string& frame_id
) {
	visualization_msgs::MarkerArray marker_array;

	for (const auto& grp: groups) {
		visualization_msgs::Marker group_marker;
		group_marker.action = visualization_msgs::Marker::ADD;
		group_marker.ns = "groups";
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
			// make it visible
			member_marker.pose.position.z += 0.1;
			member_marker.action = visualization_msgs::Marker::ADD;
			member_marker.type = visualization_msgs::Marker::CYLINDER;
			member_marker.ns = "members";
			member_marker.id = std::stoi(member.getName());
			member_marker.color.a = member.getReliability();
			member_marker.color.r = 0.9;
			member_marker.color.g = 0.2;
			member_marker.color.b = 0.2;
			member_marker.lifetime = ros::Duration(1.0);
			member_marker.scale.x = 0.30;
			member_marker.scale.y = 0.30;
			member_marker.scale.z = 1.6;

			marker_array.markers.push_back(member_marker);
		}

		visualization_msgs::Marker cog_marker;
		cog_marker.action = visualization_msgs::Marker::ADD;
		cog_marker.ns = "cog";
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
	// convert to custom types
	people_msgs_utils::People ppl;
	people_msgs_utils::Groups grp;
	std::tie(ppl, grp) = people_msgs_utils::createFromPeople(people.people);
	// create markers
	auto markers_groups = createMarkers(grp, people.header.frame_id);

	// publish
	pub.publish(markers_groups);
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "people_visualization");
	ros::NodeHandle nh;
	ros::Subscriber sub = nh.subscribe("/people", 5, peopleCallback);
	pub = nh.advertise<visualization_msgs::MarkerArray>("/people/markers", 5);
	ros::spin();
	return 0;
}
