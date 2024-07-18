#!/bin/bash


# 获取主机用户的 uid 和 gid
#echo "wjx $uid ,$gid ,$user"
uid=${uid:-"10000"}
gid=${gid:-"10000"}
user=${user:-"k230"}
home=${home:-"/home/${user}"}

echo "wjx $uid ,$gid ,$user"
#env
# 创建用户组和用户
#echo "$(id),$(pwd)"
groupadd --gid $gid usergroup
useradd --uid $uid --gid $gid -d $home --shell /bin/bash ${user}

su ${user}
