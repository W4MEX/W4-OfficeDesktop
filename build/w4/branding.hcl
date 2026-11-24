# W4 branding overlay. Base orchestration and internal package paths are retained.
target "_common" {
  args = { THEME = "w4-office" }
}
target "desktop-js" {
  args = { COMPANY_NAME = "W4", APP_TITLE_TEXT = "W4 Office", HELP_URL = "https://github.com/W4MEX" }
}
