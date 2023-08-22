#!/bin/bash

protoc sem_features.proto --cpp_out=.
protoc sem_features.proto --python_out=.
protoc branch_info.proto --cpp_out=.
protoc branch_info.proto --python_out=.

cp sem_features.pb.h ../qemu-6.2.0/linux-user/pr_semantic/
cp sem_features.pb.cc ../qemu-6.2.0/linux-user/pr_semantic/
cp branch_info.pb.cc ../qemu-6.2.0/linux-user/pr_semantic/
cp branch_info.pb.h ../qemu-6.2.0/linux-user/pr_semantic/