#!/bin/zsh -eu

# PROJECT
# -----------------------------------
PROJECT=$(realpath $0:h)
cd "$PROJECT"

zparseopts -D -E -F -- \
           {h,-help}=help  \
           -qmk-home:=qmk_home \
           -via-app-home:=via_app_home \
           {u,-update-via-app}=update_via_app \
           {w,-without-generate}=without_generate \
  || return


local -A KEYBOARDS=(
  neko60     bakeneko60
  ciel60     ciel60
  d60        dz60rgb_wkl_v2_1_atmel_dfu
  fk680      fk680pro_v2
  ikki68     ikki68_aurora
  k6         k6_pro_ansi_rgb
  libra      libra_mini
  prime_e    prime_e_rgb
  q60        q60_ansi
  qk60       qk60
  qk65       qk65_solder
  t60        tetrix60
  wood60     dz60rgb_wkl_v2_1_atmel_dfu
  zoom65     zoom65
)

# configuration
# -----------------------------------
QMK_HOME="$HOME/Documents/Sources/qmk_firmware"
VIA_APP_HOME="$HOME/Documents/Sources/via/app"
BUILD_JSON=true

# .config
#  override configuration
# -----------------------------------
[ -s .config ] &&  source .config

# help usage
# -----------------------------------
if (( $#help )); then
  print -rC1 --      \
        "Usage:" \
        "$0:t <-h|--help>                                            help" \
        "$0:t <-u|--update-via-app> [--via-app-home VIA_APP_HOME]    update & setup via/app" \
        "$0:t [options...] TARGET                                    run via/app" \
        "" \
        "options:" \
        "  --qmk-home <QMK_HOME>            location for local qmk_firmware repository" \
        "  --via-app-home <VIA_APP_HOME>    location for local via/app repository" \
        "  -w,--without-generate            Use JSON file in dist folder without running builder" \
        "" \
        "available targets:"
  for target in ${(k)KEYBOARDS}; do
    print -rC2 -- "   ${target}:"  "${KEYBOARDS[$target]}"
  done
  return
fi

# option parameters
# -----------------------------------
(( $#qmk_home )) && QMK_HOME=${qmk_home[-1]##=}
(( $#via_app_home )) && VIA_APP_HOME=${via_app_home[-1]##=}
(( $#without_generate )) && BUILD_JSON=false

# update via/app
#______________________________________
if (( $#update_via_app )); then
  cd "$VIA_APP_HOME"

  git reset --hard HEAD
  git clean -dfx
  git pull

  for patch in $(ls -v "${PROJECT}/patches/"via_app_*.patch); do
    patch --verbose -p1 < $patch
  done

  yarn install
  return
fi


if [ $# = 0 ]; then
  print -r "Error: Missing target argument." >&2
  exit 1
fi
if [ $# != 1 ]; then
  # becuase all my keyboards are assigned same Product/Vendor ID in mac mode.
  print -r "Error: Only one target is allowed." >&2
  exit 1
fi
TARGET=$@
MAKE_TARGET=$KEYBOARDS[$TARGET]


# generate JSON
#______________________________________
if $BUILD_JSON; then
  mkdir -p dist
  # generate via json file
  QMK_HOME="$QMK_HOME" \
    "$PROJECT/util/generate_via_json.js" $MAKE_TARGET
fi

# clean JSON files in via-keyboards
#______________________________________
rm -rf node_modules/via-keyboards/src
rm -rf node_modules/via-keyboards/v3
mkdir node_modules/via-keyboards/src
mkdir node_modules/via-keyboards/v3

# copy JSON files into via-keyboards
#______________________________________
OUTPUT_DIR=node_modules/via-keyboards/v3
cp "dist/${MAKE_TARGET//\//_}_via"*.json "$OUTPUT_DIR"

# convert JSON to via/app definition
#______________________________________
rm -rf "${VIA_APP_HOME}/public/definitions"

# TODO via-keybopards command rimraf dist folder
mv dist evacuate_dist
npx via-keyboards "${VIA_APP_HOME}/public/definitions"
mv evacuate_dist dist

# start VIA
#______________________________________
cd "$VIA_APP_HOME"

yarn start
