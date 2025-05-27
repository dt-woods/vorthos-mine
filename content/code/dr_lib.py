import os
import shutil
import subprocess
import sys

def download_header_from_github(
    repo_url: str,
    header_path_in_repo: str,
    destination_dir: str = ".",
    temp_dir_prefix: str = "temp_repo_"
) -> bool:
    """
    Clones a Git repository, copies a specific header file, and then deletes the cloned repository.

    Args:
        repo_url: The URL of the Git repository (e.g., "https://github.com/mackron/miniaudio.git").
        header_path_in_repo: The path to the header file *within* the repository
                             (e.g., "miniaudio.h" or "dr_wav/dr_wav.h").
        destination_dir: The directory where the header file should be copied.
                         Defaults to the current working directory.
        temp_dir_prefix: Prefix for the temporary directory created for cloning.

    Returns:
        True if the header was successfully downloaded and moved, False otherwise.
    """
    repo_name = repo_url.split('/')[-1].replace('.git', '')
    temp_clone_dir = os.path.join(temp_dir_prefix + repo_name)
    header_source_path = os.path.join(temp_clone_dir, header_path_in_repo)
    header_destination_path = os.path.join(destination_dir, os.path.basename(header_path_in_repo))

    print(f"--- Attempting to download '{os.path.basename(header_path_in_repo)}' ---")
    print(f"  Source Repo: {repo_url}")
    print(f"  Destination: {os.path.abspath(header_destination_path)}")

    # 1. Check for Git installation
    try:
        subprocess.run(["git", "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Error: Git is not installed or not found in PATH. Please install Git.")
        return False

    # 2. Clone the repository
    try:
        if os.path.exists(temp_clone_dir):
            print(f"  Warning: Temporary directory '{temp_clone_dir}' already exists. Removing it.")
            shutil.rmtree(temp_clone_dir)

        print(f"  Cloning '{repo_url}' into '{temp_clone_dir}'...")
        # We use depth 1 to only get the latest commit, saving time and space
        subprocess.run(["git", "clone", "--depth", "1", repo_url, temp_clone_dir], check=True)
        print("  Cloning successful.")
    except subprocess.CalledProcessError as e:
        print(f"Error during git clone: {e}")
        print(f"  Stderr: {e.stderr.decode()}")
        return False
    except Exception as e:
        print(f"An unexpected error occurred during cloning: {e}")
        return False

    # 3. Copy the header file
    try:
        if not os.path.exists(header_source_path):
            print(f"Error: Header file '{header_path_in_repo}' not found in cloned repository.")
            print(f"  Expected path: {os.path.abspath(header_source_path)}")
            return False

        print(f"  Copying '{header_source_path}' to '{header_destination_path}'...")
        shutil.copy(header_source_path, header_destination_path)
        print("  Copy successful.")
        return True
    except FileNotFoundError:
        print(f"Error: Source header '{header_source_path}' not found. Check header_path_in_repo.")
        return False
    except Exception as e:
        print(f"An error occurred during file copying: {e}")
        return False
    finally:
        # 4. Delete the temporarily cloned repo (always attempt to clean up)
        if os.path.exists(temp_clone_dir):
            print(f"  Cleaning up temporary directory '{temp_clone_dir}'...")
            try:
                shutil.rmtree(temp_clone_dir)
                print("  Cleanup successful.")
            except Exception as e:
                print(f"Warning: Could not remove temporary directory '{temp_clone_dir}': {e}")


if __name__ == "__main__":
    # Define the headers we need
    headers_to_download = [
        {
            "repo_url": "https://github.com/mackron/miniaudio.git",
            "header_path_in_repo": "miniaudio.h"
        },
        {
            "repo_url": "https://github.com/mackron/dr_libs.git",
            "header_path_in_repo": "dr_wav.h"
        }
        # Add other headers if you need them, e.g.:
        # {
        #     "repo_url": "https://github.com/mackron/dr_libs.git",
        #     "header_path_in_repo": "dr_mp3/dr_mp3.h"
        # }
    ]

    all_successful = True
    for header_info in headers_to_download:
        success = download_header_from_github(
            header_info["repo_url"],
            header_info["header_path_in_repo"]
        )
        if not success:
            all_successful = False
            print(f"\n--- Failed to download {os.path.basename(header_info['header_path_in_repo'])}. Aborting further downloads. ---\n")
            # If one fails, you might want to stop or continue.
            # Here, it continues to try others but marks the overall run as failed.
            break # Or 'continue' if you want to attempt all even if one fails

    if all_successful:
        print("\nAll specified header files downloaded successfully!")
    else:
        print("\nSome header files failed to download. Check the errors above.")
        sys.exit(1) # Exit with an error code
