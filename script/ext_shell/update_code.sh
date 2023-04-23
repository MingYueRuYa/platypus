#!/bin/sh

RED='\e[1;31m' # 红
RES='\e[0m'

GREEN='\033[32m'
GREEN_END='\033[0m'

function echo_red {
	echo -e "${RED}************$1************${RES}"
}

function echo_green {
	echo -e "${GREEN}************$1************${GREEN_END}"
}

stashed=0
config_file="update_config.json"

while getopts ":h" opt; do
  case $opt in
    h)
	  echo "usage:./update_code.sh [config_file]"
      exit 0
      ;;
  esac
done

if [ $# -ne 0 ]; then
	config_file=$1
fi

remote_svr="origin"
remote_branch="master"

# 1 读取配置文件
if [ -f $config_file ]; then
	while IFS='=' read -r key value; do
		if [ $key = "remote" ]; then
			remote_svr=$value
		fi
		if [ $key = "branch" ]; then
			remote_branch=$value
		fi
	done < $config_file
else
	echo_green "Not find $config_file, use default parameters"
fi

echo_green "remote server:"$remote_svr
echo_green "remote branch name:"$remote_branch

echo ""

# 获取git remote出来的远程服务器名称，并将多个名称放入数组中
# remote_names=($(git remote))

# 输出数组中的所有元素
# for name in "${remote_names[@]}"
# do
#   echo "The remote server name is: $name"
# done

# 2 查看是否有文件，子模块修改
echo_red "git status start"
status=$(git status --porcelain -uno)

if [ -n "$status" ]; then
	echo_red "modified files"
	echo "$status"
	echo_red "modified files"

	echo ""

	echo_red "git stash start"
	git stash 
	echo_red "git stash end"
	echo ""

	stashed=1

	# 如果有3rd，hc字段，表示有子模块更新
	# 第三方子仓库的目录组织方式，必须如下: .../3rd/  .../hc/
	if echo "$status" | grep -qE "idl|3rd|.premake"; then
		echo_red "update submodule starts"
		git submodule update --init --recursive
		echo_red "update submodule end"
		git stash 
	fi
else
	echo_green "Not find any modified."
fi
echo_red "git status end"

echo ""

# 远程服务器的名称，可能存在多个，
# 所以约定成俗，上游的服务器统称为up(upstream)
# 3 拉取指定服务器、分支代码
echo_red "git fetch $remote_svr start"
git fetch $remote_svr
echo_red "git fetch $remote_svr end"

echo ""

# 远程服务器的分支格式必须是统一的。
# 如：up/release/branch_name，从本地的分支获取名字，组成远程分支名称
echo_red "git rebase start"
# 这种方式要求本地分支和远程名称一致，且满足特定的格式。
# 条件苛刻，故不再采用，而是采用将分支名通过参数传递
# branch_name=$(git rev-parse --abbrev-ref HEAD)
# version=$(basename "$branch_name")
git rebase "$remote_svr/$remote_branch"

# 4 检查rebase过程中是否出现错误
if [ $? -ne 0 ]; then
	echo_green "git rebase occur error.Please rebase manual."
	echo_red "git rebase end"
	if [ $stashed -eq 1 ]; then
		echo_green "you maybe git stash pop manual."
	fi
	exit 1
fi

echo_red "git rebase end"

echo ""

# 5 判断是否需要stash pop
if [ $stashed -eq 1 ]; then
	echo_red "git stash pop start"
	git stash pop
	echo_red "git stash pop end"
fi
