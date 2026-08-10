from setuptools import setup, find_packages
from setuptools.command.install import install
import subprocess
import urllib.request
import tarfile
import os

# GitHub Release URL (உங்களது லேட்டஸ்ட் வெர்ஷன்டட் டார் கோப்பு அல்லது ரிலீஸ்)
GITHUB_REPO_URL = "https://github.com/BackendDeveloperHub/bdh-terminal-engine/archive/refs/tags/0.0.1.tar.gz"

class PostInstallCommand(install):
    """Post-installation for installation mode. (PyPI-ல் இன்ஸ்டால் ஆனதும் C இன்ஜினை பில்ட் செய்யும்)"""
    def run(self):
        install.run(self)
        print("\n[BDH Engine] Downloading & Compiling bdh-terminal-engine from GitHub Release...")
        
        # 1. டவுன்லோட் செய்தல்
        tar_path = "/tmp/bdh-engine.tar.gz"
        urllib.request.urlretrieve(GITHUB_REPO_URL, tar_path)
        
        # 2. எக்ஸ்ட்ராக்ட் செய்தல்
        extract_dir = "/tmp/bdh-terminal-engine-src"
        os.makedirs(extract_dir, exist_ok=True)
        with tarfile.open(tar_path, "r:gz") as tar:
            tar.extractall(path=extract_dir)
            
        # 3. Make & Install செய்தல்
        # (கவனிக்க: சோர்ஸ் உள்ள ஃபுல்டருக்குச் சென்று make & make install இயக்குவது)
        extracted_folder = [os.path.join(extract_dir, d) for d in os.listdir(extract_dir) if os.path.isdir(os.path.join(extract_dir, d))][0]
        
        subprocess.run(["make", "clean"], cwd=extracted_folder, check=True)
        subprocess.run(["make"], cwd=extracted_folder, check=True)
        
        # சிஸ்டம் பார்த்-க்கு காப்பி செய்தல் (Global Command)
        binary_path = os.path.join(extracted_folder, "bdh-engine")
        dest_path = "/usr/local/bin/bdh-terminal-engine"
        
        if os.path.exists(binary_path):
            subprocess.run(["sudo", "cp", binary_path, dest_path], check=True)
            print("\n[BDH Engine] 🚀 Successfully installed! You can now run 'bdh-terminal-engine' anywhere.")
        else:
            print("\n[Error] Compilation failed. Please check GCC/Make tools.")

setup(
    name="bdh-terminal-engine",
    version="1.0.0",
    description="A 100% Pure Linux CLI Terminal Multiplexer written in C.",
    long_description=open("README.md", encoding="utf-8").read(),
    long_description_content_type="text/markdown",
    author="Prabakaran",
    author_email="your-email@example.com",
    url="https://github.com/BackendDeveloperHub/bdh-terminal-engine",
    packages=find_packages(),
    cmdclass={
        'install': PostInstallCommand,
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: POSIX :: Linux",
        "Topic :: Terminals :: Terminal Emulators/X-Terminals",
    ],
    python_requires='>=3.6',
)
