import os
import re
import sys
import argparse
from shutil import copyfile


def copy_file(path_read, path_write, i):
    names = os.listdir(path_read)
    for name in names:
        path_read_new = os.path.join(path_read, name)
        path_write_new = os.path.join(path_write, name)
        if os.path.isdir(path_read_new):
            if not os.path.exists(path_write_new):
                os.makedirs(path_write_new)
            copy_file(path_read_new, path_write_new, i)
        else:
            copyfile(path_read_new, path_write_new)
            if('0' in path_write_new or name == 'boundary'):
                modify_file(path_write_new, i)


def modify_file(file_name, i):
    shift = phy_num*(i//phy_num)
    with open(file_name, "r") as f:
        lines = f.readlines()
    with open(file_name, "w") as f_w:
        for line in lines:
            if "procBoundary" in line or "myProcNo" in line or "neighbProcNo"in line:
                line = re.sub('\d+', lambda x: str(int(x.group())+shift), line)
	    if "numberOfSubdomains" in line:
                line = re.sub('\d+',str(i),line)
            f_w.write(line)


   

if __name__ == "__main__":
    phy_num = 1
    vel_num = 1
    root = os.getcwd()
    dict_root=os.path.join(root,'system','decomposeParDict')

#need rm -f to remove the existing processorx folder
    os.system("rm -r processor*")

    parser = argparse.ArgumentParser(description='decompose physics space used for phy&vel parallel.')
    parser.add_argument('-p', required=True, help="the number of physical subdomains one wish to decompose", type=int)
    parser.add_argument('-v', required=True, help="the number of velocity subdomains one wish to decompose", type=int)
    args = parser.parse_args()
    phy_num = args.p
    vel_num = args.v
   
    #modify numofsubdomains = phy_num in decomposePar
    modify_file(dict_root,phy_num)

    #create the first row
    os.system("decomposePar")
    
    #create the rest processors folders
    for i in range(phy_num, phy_num * vel_num):
        path_read = os.path.join(root, 'processor'+str(i % phy_num))
        path_write = os.path.join(root, 'processor'+str(i))
        copy_file(path_read, path_write, i)
    
    #modify numofsubdomains = phy_num*vel_num in decomposePar
    modify_file(dict_root,phy_num*vel_num)
    
    #copy vMesh
    os.system("echo ./processor*/constant | xargs -n 1 cp -r ./constant/vMesh")

    

	

