# Usage:
#	rpm -ba allinanna.spec.in
# Precautions:
#	/usr/src/redhat/SOURCES/magic-VERSION.tar.gz
#	/usr/src/redhat/SOURCES/inanna-VERSION.tar.gz
#
# They must match the versions given here:
%define inannaversion 0.3.2
%define magicversion 1.0.3
%define kauditorversion 0.3

################################################################################
%define version %{inannaversion}
Summary: Artificial neural network C++ library
Name: inanna
Version: %{version}
Release: 1
Copyright: LGPL
Packager: Marko Grönroos <magi@iki.fi>
Group: Development/Libraries
URL: http://inanna.sourceforge.net/
Prefix: /usr
requires: magic = %{magicversion}

Source0: inanna-%{inannaversion}.tar.gz
Source1: magic-%{magicversion}.tar.gz
Source2: kauditor-%{kauditorversion}.tar.gz
BuildRoot: /tmp/%{name}-buildroot

%description
Inanna is a C++ library for neural learning and computation. It
provides object-oriented network and data handling, with many
interesting features.

Included in the source package are also some interesting test
projects.

########################################
%package -n magic
Summary: Artificial neural network C++ library
Version: %{magicversion}
Release: 1
Group: Development/Libraries

%description -n magic
Base lib

########################################
%package -n kauditor
Summary: Auditing tool with KDE GUI
Version: %{kauditorversion}
Release: 1
Group: Applications/Productivity

%description -n kauditor
Auditing tool

################################################################################
%prep
%setup -q -D -c -T -a 0
%setup -q -D -c -T -a 1
%setup -q -D -c -T -a 2

################################################################################
%build
export COMMONBASE=$RPM_BUILD_DIR/%{name}-%{inannaversion}

# Compile magic
cd magic-%{magicversion}
#./configure --prefix=%{prefix} --with-install-root=$RPM_BUILD_ROOT/magic
#make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"
cd ..

# Compile inanna
# We read the includes and libraries directly from the freshly compiled magiclib

cd inanna-%{inannaversion}
#./configure --prefix=%{prefix} --with-install-root=$RPM_BUILD_ROOT/magic --with-extra-includes=$COMMONBASE/magic-%{magicversion}
#make RPM_OPT_FLAGS="$RPM_OPT_FLAGS -I$COMMONBASE/magic-%{magicversion} -L$COMMONBASE/magic-%{magicversion}/magic"
rm -f inanna
ln -s src inanna
cd ..

# Compile kauditor
cd kauditor-%{kauditorversion}
export KDEDIR=%{prefix}
./configure --prefix=%{prefix} --with-install-root=$RPM_BUILD_ROOT/magic --with-extra-includes=$COMMONBASE/magic-%{magicversion}:$COMMONBASE/inanna-%{inannaversion}:$COMMONBASE/inanna-%{inannaversion}/src:$COMMONBASE/inanna-%{inannaversion}/proj/auditing
make
# We want exceptions! RPM_OPT_FLAGS="$RPM_OPT_FLAGS"
cd ..

################################################################################
%install
export COMMONBASE=$RPM_BUILD_DIR/%{name}-%{inannaversion}
rm -rf $RPM_BUILD_ROOT

function installpackage () {
	export PACKAGE=$1
	export PACKAGEVERSION=$2

	cd $COMMONBASE/${PACKAGE}-${PACKAGEVERSION}
	make DESTDIR=$RPM_BUILD_ROOT/${PACKAGE} install

	# Store filenames
	cd $RPM_BUILD_ROOT/${PACKAGE}
	find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $COMMONBASE/file.list.${PACKAGE}
	find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $COMMONBASE/file.list.${PACKAGE}
	find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $COMMONBASE/file.list.${PACKAGE}
	cp -r * ..
	rm -rf ${PACKAGE}
}

# Install
installpackage magic %{magicversion}
installpackage inanna %{inannaversion}
installpackage kauditor %{kauditorversion}

# Move the separate paths to common path
cd $RPM_BUILD_ROOT
rm -rf magic inanna kauditor

################################################################################
%clean
rm -rf $RPM_BUILD_ROOT

%changelog -n inanna

%files -f file.list.inanna
%files -n magic -f file.list.magic
%files -n kauditor -f file.list.kauditor
