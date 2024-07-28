# vorthos-mine
The 'Vorthos  Mine' website is built using '[blogdown][blogdown_web]', an R-based website builder based on RStudio's '[bookdown][bookdown_web]' written in markdown.

## Installation
1. Install [pandoc](https://pandoc.org/)
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

    For clones of this repository, remember to run the following:

    ```sh
    git submodule init
    git submodule update
    ```

    in order to get the 'mainroads' theme copied to the repo.
1. Download the blogdown R-package (e.g., version 1.9)

    ```r
    > install.packages("blogdown")
    ```

1. Build/serve site.

    ```r
    blogdown::serve_site()
    ```

    ```r
    blogdown::build_site()
    ```


[blogdown_web]: https://bookdown.org/yihui/blogdown/
[bookdown_web]: https://bookdown.org/


---

# Appendix

CSV color scheme:

- Main (metallic gold): d4af37
- Accent-1 (metallic sunburst): 9c7c38
- Accent-2 (pearl): eae0c8
