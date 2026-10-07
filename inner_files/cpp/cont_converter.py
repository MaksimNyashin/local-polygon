from os import path
from sys import argv
from shutil import make_archive

def main():
    home_dir = path.dirname(path.dirname(path.dirname(path.abspath(__file__))))
    cont_dir = path.join(home_dir, "cont")
    arch_dir = path.join(cont_dir, "to_arch")
    for arg in argv[1:]:
        with open(path.join(arch_dir, arg), "r", encoding="utf-8") as fi:
            text = fi.read()
        with open(path.join(arch_dir, arg), "w", encoding="cp1251") as fo:
            fo.write(text)
    make_archive(path.join(cont_dir, "tmp"), "zip", root_dir=arch_dir)

if __name__ == "__main__":
    main()
