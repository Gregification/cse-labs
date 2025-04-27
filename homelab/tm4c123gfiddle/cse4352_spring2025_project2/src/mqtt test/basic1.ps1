# bash odsent seem to like it for some reason, so this instead 

# subscribe example
# mosquitto_sub -h 127.0.0.1 -t "test topic"

# post example
# mosquitto_pub -h 127.0.0.1 -t "test topic" -m "ping pong"

mosquitto -c ./basic1.conf -v
