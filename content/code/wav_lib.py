import os
import subprocess
import shutil
import json
import sys
import tempfile # Needed for creating temporary files

# --- Configuration ---
WAVEFORM_LIBRARY_ROOT = "/home/user/Dropbox/Music/waveform_library" # <<< IMPORTANT: SET THIS TO YOUR ACTUAL PATH
TARGET_SAMPLE_RATE = 44100  # Hz (consistent with your C++ project's SAMPLE_RATE)
TARGET_CHANNELS = 1         # 1 for mono, 2 for stereo
OVERWRITE_ORIGINAL = True   # Set to True to overwrite, False to save converted files in a new directory
                            # If False, ensure OUTPUT_DIR is set and distinct from WAVEFORM_LIBRARY_ROOT
OUTPUT_DIR = "conformed_waveform_library" # Used only if OVERWRITE_ORIGINAL is False
BACKUP_ORIGINAL_FILES = True # Creates a '.bak' backup before conversion if OVERWRITE_ORIGINAL is True

# --- Loudness Normalization Settings ---
# -23.0 LUFS is the EBU R 128 standard for broadcast.
# -18.0 LUFS is often used for music, offering more perceived "punch."
# Choose what sounds best for your library.
TARGET_LOUDNESS_LUFS = -18.0 # Integrated Loudness target in LUFS
TARGET_TRUE_PEAK_DBFS = -2.0 # Max True Peak target in dBFS (e.g., -2dBFS for headroom)
TARGET_LOUDNESS_RANGE = 7.0  # Target Loudness Range in LU (e.g., 7 LU for music)


# --- Check for FFmpeg installation ---
def check_ffmpeg():
    try:
        subprocess.run(["ffmpeg", "-version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        subprocess.run(["ffprobe", "-version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print("FFmpeg and ffprobe found.")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Error: FFmpeg (or ffprobe) is not installed or not found in your system's PATH.")
        print("Please install it. On Linux Mint, you can typically do this with: sudo apt install ffmpeg")
        return False


def check_ffprobe():
    try:
        subprocess.run(["ffprobe", "-version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print("ffprobe found.")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Error: ffprobe is not installed or not found in your system's PATH.")
        print("Please install FFmpeg. On Linux Mint, you can typically do this with: sudo apt install ffmpeg")
        return False


def get_wav_info(filepath):
    """Uses ffprobe to get sample rate and channel count of a WAV file."""
    try:
        cmd = [
            "ffprobe",
            "-v", "quiet",
            "-print_format", "json",
            "-show_streams",
            filepath
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        info = json.loads(result.stdout)

        for stream in info.get("streams", []):
            if stream.get("codec_type") == "audio":
                sample_rate = int(stream.get("sample_rate", 0))
                channels = int(stream.get("channels", 0))
                return sample_rate, channels
        return 0, 0 # No audio stream found
    except (subprocess.CalledProcessError, json.JSONDecodeError, FileNotFoundError) as e:
        print(f"Error analyzing {filepath}: {e}")
        return -1, -1 # Indicate error with -1 to differentiate from '0' (no audio stream)


def conform_wav_file(filepath, target_sr, target_channels, output_path_final, backup_original=False):
    """Converts a WAV file to the target sample rate and channel count, and normalizes loudness using FFmpeg."""
    current_sr, current_channels = get_wav_info(filepath)

    if current_sr == -1: # Error during analysis
        print(f"Skipping {filepath}: Error during initial analysis.")
        return False
    elif current_sr == 0: # No audio stream detected
        print(f"Skipping {filepath}: No audio stream found.")
        return False

    # Check if the file already conforms to both sample rate, channels, AND loudness (approximately)
    # Re-running loudnorm on already conforming files is generally okay, but can be skipped for speed.
    # For simplicity, we'll re-process loudness if SR/channels already conform.
    if current_sr == target_sr and current_channels == target_channels:
        print(f"Processing {filepath}: SR: {current_sr}Hz, Ch: {current_channels} (already conforming, re-normalizing loudness).")
    else:
        print(f"Conforming {filepath} (Original: {current_sr}Hz, {current_channels}ch) to ({target_sr}Hz, {target_channels}ch) and normalizing loudness...")


    # If overwriting, create a temporary output file first
    temp_output_path = None
    if output_path_final == filepath:
        fd, temp_output_path = tempfile.mkstemp(suffix=".wav", dir=os.path.dirname(filepath))
        os.close(fd) # Close the file descriptor immediately
        print(f"  Using temporary file: {temp_output_path}")
    else:
        temp_output_path = output_path_final # If not overwriting, output directly to final path

    # Create backup if needed (this now happens before ffmpeg conversion)
    if backup_original and output_path_final == filepath: # Only backup if we are overwriting the original
        backup_path = filepath + ".bak"
        if os.path.exists(backup_path):
            print(f"  Backup '{backup_path}' already exists. Skipping backup creation.")
        else:
            print(f"  Creating backup: {backup_path}")
            try:
                shutil.copy2(filepath, backup_path) # copy2 preserves metadata
            except Exception as e:
                print(f"  Error creating backup for {filepath}: {e}. Skipping backup for this file.")
                # Decide if you want to fail here or proceed without backup.
                # For robustness, we'll proceed for now, but a warning is logged.

    # FFmpeg command
    # ... (rest of the FFmpeg command definition is unchanged) ...
    cmd = [
        "ffmpeg",
        "-i", filepath,
        "-ar", str(target_sr),
        "-ac", str(target_channels),
        "-y", # Overwrite output file if it exists
        "-map_metadata", "-1", # Remove all metadata
        "-c:a", "pcm_f32le", # Use 32-bit float PCM
        "-filter:a", f"loudnorm=I={TARGET_LOUDNESS_LUFS}:TP={TARGET_TRUE_PEAK_DBFS}:LRA={TARGET_LOUDNESS_RANGE}",
        temp_output_path # Output to the temporary file or the designated output_path
    ]

    try:
        subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='utf-8')
        print(f"Successfully converted to temporary file {temp_output_path}")

        # If we converted to a temporary file, now replace the original
        if output_path_final == filepath:
            try:
                os.replace(temp_output_path, output_path_final) # Atomic rename (more robust than remove+rename)
                print(f"Replaced original {output_path_final} with conformed version.")
                return True
            except Exception as e:
                print(f"Error replacing original file {output_path_final} with conformed version: {e}")
                # Clean up temp file if replacement fails
                if os.path.exists(temp_output_path):
                    os.remove(temp_output_path)
                return False
        else:
            return True # Successfully converted to the designated output_path_final

    except subprocess.CalledProcessError as e:
        print(f"Error conforming {filepath}:")
        print(f"  Command: {' '.join(cmd)}")
        print(f"  FFmpeg stdout: {e.stdout}")
        print(f"  FFmpeg stderr: {e.stderr}")
        # Clean up temp file if ffmpeg fails
        if temp_output_path and os.path.exists(temp_output_path):
            os.remove(temp_output_path)
        return False
    except FileNotFoundError:
        print("Error: FFmpeg command not found. Ensure FFmpeg is installed and in your PATH.")
        return False
    except Exception as e:
        print(f"An unexpected error occurred during conversion for {filepath}: {e}")
        # Clean up temp file if any other error occurs
        if temp_output_path and os.path.exists(temp_output_path):
            os.remove(temp_output_path)
        return False


def main():
    if not check_ffmpeg():
        sys.exit(1)

    if not os.path.isdir(WAVEFORM_LIBRARY_ROOT):
        print(f"Error: Waveform library root '{WAVEFORM_LIBRARY_ROOT}' does not exist or is not a directory.")
        sys.exit(1)

    # Validate output_dir if not overwriting
    if not OVERWRITE_ORIGINAL:
        if os.path.abspath(OUTPUT_DIR) == os.path.abspath(WAVEFORM_LIBRARY_ROOT):
            print(f"Error: OUTPUT_DIR cannot be the same as WAVEFORM_LIBRARY_ROOT when OVERWRITE_ORIGINAL is False.")
            sys.exit(1)
        os.makedirs(OUTPUT_DIR, exist_ok=True) # Ensure output directory exists

    print(f"\n--- Starting WAV library conformation ---")
    print(f"Target Sample Rate: {TARGET_SAMPLE_RATE} Hz")
    print(f"Target Channels: {TARGET_CHANNELS} (1=Mono, 2=Stereo)")
    print(f"Library Root: {os.path.abspath(WAVEFORM_LIBRARY_ROOT)}")
    if OVERWRITE_ORIGINAL:
        print(f"Action: Overwriting original files.")
        if BACKUP_ORIGINAL_FILES:
            print(f"  (Backup will be created: .bak extension)")
    else:
        print(f"Action: Saving conformed files to new directory: {os.path.abspath(OUTPUT_DIR)}")

    files_processed = 0
    files_conformed_or_skipped = 0
    files_failed = 0

    for root, _, files in os.walk(WAVEFORM_LIBRARY_ROOT):
        for file in files:
            if file.lower().endswith(".wav"):
                filepath = os.path.join(root, file)
                files_processed += 1

                # Determine the final output path (either original path or path in new output dir)
                if OVERWRITE_ORIGINAL:
                    output_path_final = filepath
                else:
                    relative_path = os.path.relpath(filepath, WAVEFORM_LIBRARY_ROOT)
                    output_path_final = os.path.join(OUTPUT_DIR, relative_path)
                    os.makedirs(os.path.dirname(output_path_final), exist_ok=True) # Ensure subdir exists

                # Call the conformity function
                if conform_wav_file(filepath, TARGET_SAMPLE_RATE, TARGET_CHANNELS, output_path_final, BACKUP_ORIGINAL_FILES):
                    files_conformed_or_skipped += 1
                else:
                    files_failed += 1

    print("\n--- Conformation Summary ---")
    print(f"Total WAV files found: {files_processed}")
    print(f"Files Conformed (or already conforming): {files_conformed_or_skipped}")
    print(f"Files Failed: {files_failed}")

    if files_failed > 0:
        print("\nSome files failed to conform. Please check the error messages above.")
        sys.exit(1)
    else:
        print("\nWAV library conformation completed successfully!")


def main_dry_run():
    if not check_ffprobe():
        sys.exit(1)

    if not os.path.isdir(WAVEFORM_LIBRARY_ROOT):
        print(f"Error: Waveform library root '{WAVEFORM_LIBRARY_ROOT}' does not exist or is not a directory.")
        sys.exit(1)

    print(f"\n--- Starting WAV library audit (DRY RUN) ---")
    print(f"Scanning Library Root: {os.path.abspath(WAVEFORM_LIBRARY_ROOT)}\n")
    print(f"{'File Path':<80} | {'Sample Rate':>12} | {'Channels':>8}")
    print(f"{'-'*80}-+-{'-'*12}-+-{'-'*8}")

    files_found = 0
    files_analyzed = 0
    files_with_errors = 0

    for root, _, files in os.walk(WAVEFORM_LIBRARY_ROOT):
        for file in files:
            if file.lower().endswith(".wav"):
                filepath = os.path.join(root, file)
                files_found += 1

                sr, ch = get_wav_info(filepath)

                if sr == -1: # Error during analysis
                    print(f"{filepath:<80} | {'ERROR':>12} | {'ERROR':>8}")
                    files_with_errors += 1
                elif sr == 0: # No audio stream detected
                    print(f"{filepath:<80} | {'NO AUDIO':>12} | {'N/A':>8}")
                    files_with_errors += 1
                else:
                    print(f"{filepath:<80} | {sr:>12} | {ch:>8}")
                    files_analyzed += 1

    print(f"\n{'-'*80}-+-{'-'*12}-+-{'-'*8}")
    print("\n--- Audit Summary ---")
    print(f"Total WAV files found: {files_found}")
    print(f"Files successfully analyzed: {files_analyzed}")
    print(f"Files with errors or no audio stream: {files_with_errors}")
    print("\nThis was a DRY RUN. No files were modified.")


if __name__ == "__main__":
    # main_dry_run()
    main()