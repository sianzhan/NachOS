# Report
## 目錄
1. 嘗試學習自己實作一個 system call function
2. 嘗試使用debug
3. 實作system call sleep function
4. 總結感想

## 1. 嘗試學習自己實作一個 system call function
問題：
1. 沒辦法在test/目錄夾底下使用make指令
    ![](https://i.imgur.com/6841G6Q.png)
    如图所示我的/usr/local/nachos/decstation-ultrix/bin/gcc    沒有 gcc 編輯器
    解決的方法->
    - (失敗)一開始我是直接把Makefile裏面的路徑强行設成我gcc的位子，不過結果是失敗了
    ![](https://i.imgur.com/oRgw7Sq.png)

        ![](https://i.imgur.com/7jSSUB6.png)


    - (成功)把相關字眼做個查詢就有能夠下載的[網站](https://uw.cs.cs350.narkive.com/n09pzD3U/installing-nachos-on-linux)以及其[説明](https://www.student.cs.uwaterloo.ca/~cs350/common/linux-nachos.html)，下載好之後，把路徑改一改就能使用了
    ![](https://i.imgur.com/RUir5To.png)

        ![](https://i.imgur.com/HJWMfjF.png)
2. 在/usrprogram/exception.cc裏面的修改沒辦法影響到/test/目錄底下的exe file，如下圖
    ![](https://i.imgur.com/VyAniUq.png)
    它只會偵測syscall.h的改變而已，exception.cc改變之後影響的exception.o卻不會該考慮進去
    解決方法-> 
    - (成功)到code/目錄夾底下執行make指令

3. 這個問題比較特殊，原來.h裏面的注解沒辦法使用//，只能用/**/的方法, 這個問題只會出現/test/底下的make，如果直接在/code/底下make就不會有這個問題
    ![](https://i.imgur.com/ZaOE1G7.png)
    
感想：很特別的經驗，不過，我卡在第一個問題很久，一直不斷地嘗試去查看cross-compiling究竟是什麽意思，以及gcc之後給予的選擇(gcc -G)的意義究竟是什麽~~~
## 2. 嘗試使用debug
問題：
1. 嘗試使用自己的flag，結果而言是成功的，參考[網址](http://puremonkey2010.blogspot.com/2013/03/nachos-40-debugging-nachos.html)
![](https://i.imgur.com/16Y0tCX.png)
## 3. 實作system call sleep function
問題：
1. 嘗試實作syscall sleep function，一開始是沒有概念的，即使trace完code之後也是一樣，所以只好參考網絡上的資源
[網站1](https://github.com/taldehyde/os-project-2/blob/master/report.md)
[網站2](https://morris821028.github.io/2014/05/24/lesson/hw-nachos4/)
[網站3](http://blog.terrynini.tw/tw/OS-NachOS-HW1/#System-call-Sleep)
全部相似度都很高，只是function name和class name不一樣而已，我是看了之後，才自己做出類似的(也是function name和 class name不一樣)。如果可以希望之後的作業，助教可以大概給個方向(類似給我們一個flow chart圖，或者pseudo code)，因爲就這樣而言，我也只是看了別人的做法，理解他在幹什麽，自己重複寫多一次。更希望的是，可能可以給我們一個flow chart讓我們知道一個sleep function究竟需要考慮什麽因素(schedule裏面的thread啊之類的)。可能有點强人所難吧，因爲nachos的實作練習其實也不多，沒辦法找到類似的關於syscall sleep的程式碼。
    具體一點的例子，如果沒有參考網絡資源，我會不知道把thread加入我的ThreadSleepCollection之後，要讓那個thread call Sleep(false) function。這也是我再看到別人的程式碼，覺得特別的地方，然後自己在去trace多一次code。
2. 嘗試去看官網的source code，可是第一我只找到nachos3.3的而已，第二是程式碼加總起來很多，我看了其中一個list.h和list.cc我就大概花了1個小時理解了。
[參考網站](https://www.cs.odu.edu/~cs471/soumya/sourcecode/threads/threads.html)
而且因爲他的解釋沒有example(就好像我寫好一個function，我通常在c++網站的解釋上會看到這個function的實作例子)
結果截圖：
停止途中
![](https://i.imgur.com/3IsIRoC.png)
結束
![](https://i.imgur.com/SJ4w31a.png)
## 4. 總結感想
有趣的體驗，不過，官方資源似乎太少了點，對於初學者而言，只能看解答來-回答問題。整體花費時間: 5-7小時