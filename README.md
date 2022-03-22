# vorthos-mine
The 'Vorthos  Mine' website is built using '[blogdown][blogdown_web]', an R-based website builder based on RStudio's '[bookdown][bookdown_web]' written in markdown.

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

[blogdown_web]: https://bookdown.org/yihui/blogdown/
[bookdown_web]: https://bookdown.org/
