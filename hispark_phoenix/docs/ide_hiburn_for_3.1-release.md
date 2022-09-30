# 3.1-Release分支的IDE烧录临时说明
方法一：
1. 编译源码，编译结束后将//out/hi3751v350/ramdisk.img拷贝到//out/hi3751v350/packages/phone/images/下

2. IDE导入工程

3. 替换工程根目录下./deveco/deveco.ini文件，然后重新打开窗口。

4. 连接local PC进行工程设置(设置串口，烧录方式，网口等)

5. 在Project Config中选中"Enable to use upload_partition_profile for upload; Otherwise, use the upload_partitions"
    选择"upload_partions_profile"为//out/hi3751v350/packages/phone/images/Hi3751V350-slaveboot-emmc.xml
    点击save保存。

6. 修改$(WINDOS_INSTALL)\Huawei\DevEco-Device-Tool\platforms\platform-hisilicon_arm_linux\builder\hiburn.py中的21和22行为本地java路径。
           本地java路径（可通过where java查看）。
           21行：jre_path = r"C:\Program Files (x86)\Common Files\Oracle\Java\javapath"
           22行：jre_path = jre_path if jre_path else None

    （注意：这里改动的是windows上的IDE脚本，修改一次之后就不用再修改了）

7. 点击upload进行烧录。
