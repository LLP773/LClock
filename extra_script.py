import os
from SCons.Script import Import

Import("env")

def before_buildfs(source, target, env):
    print("\n>>>  before_buildfs: build-www")
    env.Execute("ruby shared/build-www.rb")

    print("\n>>> before_buildfs: copy assets")
    env.Execute("robocopy /MIR shared\\assets data\\assets")

    print("\n>>> before_buildfs: gzip assets")
    env.Execute("powershell -Command \"Get-ChildItem data\\ -Recurse -Include *.css,*.js,*.html | ForEach-Object { gzip -Force $_.FullName }\"")

    print("\n>>> before_buildfs: total size")
    env.Execute("dir /s data")

env.AddPreAction("buildfs", before_buildfs)

def after_buildfs(source, target, env):
    print("\n>>> after_buildfs: unzip assets")
    # 如果需要在构建后解压缩文件，可以在这里添加相应的解压缩命令
    pass

env.AddPostAction("buildfs", after_buildfs)