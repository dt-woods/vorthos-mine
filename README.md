# vorthos-mine

## Installation
1. Install hugo

    For example using your favorite package manager or homebrew on macOS.
1. Create a new site (`hugo new site vorthos-mine`)

    This creates a new folder layout:

    * archetypes/
    * content/
    * data/
    * layouts/
    * static/
    * themes/
    * config.toml
1. Clone a theme to the themes directory

    For example:

    ```sh
    git submodule add https://github.com/vimux/mainroad.git themes/mainroad
    ```
1. Download the blogdown R-package (e.g., version 1.9)

    ```r
    > install.packages("blogdown")
    ```

1. Install [pandoc](https://pandoc.org/)
