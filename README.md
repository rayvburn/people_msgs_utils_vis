# people_msgs_utils_vis

Provides ROS node for visualization of data processed by [`people_msgs_utils`](https://github.com/rayvburn/people_msgs_utils) library.

Implemented nodes:

* `visualization_node` - creates markers related to human groups and their members. Currently, when a human is not assigned to any group, it will not be visualized (see [#2](https://github.com/rayvburn/people_msgs_utils_vis/issues/2)).
