【功能描述】实现自动push 大文件或者小文件
【使用环境】android4.0 及以上版本
【使用方法】
		第一：解压压缩包nand_test_V1.2.1.zip
		第二步：进入nand_test_V1.0目录
		第三步：执行命令 source setup.sh，然后根据提示“press any key to continue...”敲回车
		第四步：等待手机重启完成后，执行 adb root
【脚本修改】
		在preloadapp.sh 中定义了文件个数 以及 文件大小如下：
		./system/bin/nand_test 1 10000 100
		long long cycle    = atoi(argv[1]); // 这个就是1
		long long FileNum  = atoi(argv[2]); // 这个就是10000
		long long filesize = atoi(argv[3]); // 这个就是100
		TotalSize = filesize * FileNum ; // 10000*100 = 10M
		 这里的1 代表循环次数 10000 代表写入的文件个数 ； 100 代表写入的文件大小。
		 get_free_size("/data") < 15000000 
		当data 分区可用空间小于20M 的时候 停止数据写入并且删除存放在data 空间中的临时文件，进入下一次数据写入动作。
		需要注意：（data 空间Free 大小） - （文件个数*文件大小）<= (data 空间Free 大小)*0.3
【出错提示】
		输出log记录在 mem_test.txt
		如果运行出错手机会自动关机。


修改因为某种异常后 保存在test目录的的文件无法删除
