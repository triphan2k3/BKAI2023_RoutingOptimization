from concurrent.futures.thread import ThreadPoolExecutor
import time
import os
import sys
import argparse
    
def call_script(cmd):
    print(cmd)
    start = time.time()
    os.system(cmd)
    end = time.time()
    # print(f"Cmd done in {(end-start):.03f}s")
    
def build(file):        
    print("rm -f temp.txt")
    print('g++ -O2 -std=c++11 {}.cpp -o {}'.format(file,file))
    os.system("rm -f temp.txt")
    os.system('g++ -O2 -std=c++11 {}.cpp -o {}'.format(file,file))


def run_all(file):
    with ThreadPoolExecutor(max_workers=10) as executor:
        for i in range(1, 6):
            fileName = str(i) + '.txt'
            if i != 10:
                fileName = '0' + fileName
            cmd = "./{} ./INPUT/{} ./OUTPUT/{}".format(file,fileName, fileName)
            executor.submit(call_script, cmd)

    f = open("temp.txt", "r")
    total = 0
    for i in range(1, 6):
        ans = f.readline()
        testcase, ans = ans.split(sep=' ')
        print("{}: {}".format(testcase, ans),end="")
        total += float(ans[:-1])
    print("TOTAL: {}".format(total))
    f.close()


def run_testcase(i,file):
    filetest = str(i) + '.txt'
    if i < 10:
        filetest = '0' + filetest
    cmd = "./{} ./INPUT/{} ./OUTPUT/{}".format(file,filetest, filetest)
    os.system(cmd)
    f = open("temp.txt", "r")
    ans = f.readline()
    testcase, ans = ans.split(sep=' ')
    print("{}: {}".format(testcase, ans),end="")
    f.close()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='how to run this code')
    parser.add_argument('-f','--filename', type=str,default='main',help='File source to run')
    parser.add_argument('--n_test',choices=range(0,6),type=int,default=0)
    parser.add_argument('--run_all',action="store_true", default=False)
    
    args = parser.parse_args()   

    # if len(sys.argv) == 1:  
    #     file = 'main'
    # else:
    #     file = sys.argv[1]

    build(args.filename)
    if (args.n_test == 0) or (args.run_all):
        run_all(args.filename)
    else:
        run_testcase(args.n_test,args.filename)
    