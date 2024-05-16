@ECHO OFF
SET PACKAGE=PIDStudio
SET LANGUAGES=pl_PL
xgettext --package-name=%PACKAGE% --package-version=1.0 --msgid-bugs-address=crew37zax@gmail.com --keyword=_ --language=c++ -o locale/%PACKAGE%.pot src/%PACKAGE%.cpp
(for %%l in (%LANGUAGES%) do (
	msgmerge --update --no-fuzzy-matching locale/%%l/LC_MESSAGES/%PACKAGE%.po locale/%PACKAGE%.pot
	msgfmt --output-file=locale/%%l/LC_MESSAGES/%PACKAGE%.mo locale/%%l/LC_MESSAGES/%PACKAGE%.po
))
