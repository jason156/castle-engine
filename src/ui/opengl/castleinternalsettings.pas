{
  Copyright 2018-2019 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Reading CastleSettings.xml ( https://castle-engine.io/manual_castle_settings.php ). }
unit CastleInternalSettings;

{$I castleconf.inc}

interface

uses SysUtils, Classes, Generics.Collections, DOM, Contnrs,
  CastleUtils, CastleClassUtils, CastleFonts, CastleRectangles, CastleTimeUtils,
  CastleUIControls;

type
  TWarmupCache = class;
  TWarmupCacheFormat = class;

  EInvalidSettingsXml = class(Exception);

  TWarmupCacheFormatEvent = procedure (const Cache: TWarmupCache;
    const Element: TDOMElement; const ElementBaseUrl: String) of object;

  { Anything that can be preloaded into the TWarmupCache.
    E.g. a texture, sound, model. }
  TWarmupCacheFormat = class
    { XML element (in CastleSettings.xml) name indicating this format. }
    Name: String;
    Event: TWarmupCacheFormatEvent;
  end;

  TWarmupCacheFormatList = class({$ifdef CASTLE_OBJFPC}specialize{$endif} TObjectList<TWarmupCacheFormat>)
  private
    function CallRegisteredFormat(const Cache: TWarmupCache;
      const Element: TDOMElement; const ElementBaseUrl: String): Boolean;
  public
    procedure RegisterFormat(const Name: String; const Event: TWarmupCacheFormatEvent);
  end;

  { Used by SettingsLoad, an instance of this will be created and owner by container.
    In the future this may be available publicly (in non-internal unit) to have
    "warmpup cache" available for any period of time (right now, it is only for the
    lifetime of the container). }
  TWarmupCache = class(TComponent)
  strict private
    FOwnedObjects: TObjectList;
  private
    procedure ReadElement(const Element: TDOMElement; const ElementBaseUrl: String);
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;

    { General-purpose container for objects that should be owned by this
      cache. May be used by TWarmupCacheFormatEvent implementation,
      if you create something that should be owned by cache. }
    property OwnedObjects: TObjectList read FOwnedObjects;
  end;

{ Register new TWarmupCacheFormat using @code(WarmupCacheFormats.RegisterFormat). }
function WarmupCacheFormats: TWarmupCacheFormatList;

{ Load CastleSettings.xml ( https://castle-engine.io/manual_castle_settings.php )
  into a container.
  This also creates a TWarmupCache that is owned by the container. }
procedure SettingsLoad(const Container: TUIContainer; const SettingsUrl: String);

implementation

uses Math, TypInfo,
  CastleLog, CastleXMLUtils, CastleStringUtils, CastleGLImages;

{ TWarmupCacheFormatList ----------------------------------------------------- }

function TWarmupCacheFormatList.CallRegisteredFormat(const Cache: TWarmupCache;
  const Element: TDOMElement; const ElementBaseUrl: String): Boolean;
var
  Format: TWarmupCacheFormat;
begin
  for Format in Self do
    if Format.Name = Element.TagName8 then
    begin
      Format.Event(Cache, Element, ElementBaseUrl);
      Exit(true);
    end;
  Result := false;
end;

procedure TWarmupCacheFormatList.RegisterFormat(const Name: String;
  const Event: TWarmupCacheFormatEvent);
var
  Format: TWarmupCacheFormat;
begin
  Assert(Assigned(Event));

  Format := TWarmupCacheFormat.Create;
  Add(Format);
  Format.Name := Name;
  Format.Event := Event;
end;

{ TImageUiCache -------------------------------------------------------------- }

type
  TImageUiCache = class
    class procedure Event(const Cache: TWarmupCache;
      const Element: TDOMElement; const ElementBaseUrl: String);
  end;

class procedure TImageUiCache.Event(const Cache: TWarmupCache;
  const Element: TDOMElement; const ElementBaseUrl: String);
var
  URL: String;
  Image: TCastleImagePersistent;
begin
  URL := Element.AttributeURL('url', ElementBaseUrl);
  Image := TCastleImagePersistent.Create;
  Cache.OwnedObjects.Add(Image);
  Image.URL := URL; // loads the image
end;

{ TWarmupCache --------------------------------------------------------------- }

constructor TWarmupCache.Create(AOwner: TComponent);
begin
  inherited;
  FOwnedObjects := TObjectList.Create(true);
end;

destructor TWarmupCache.Destroy;
begin
  FreeAndNil(FOwnedObjects);
  inherited;
end;

procedure TWarmupCache.ReadElement(const Element: TDOMElement; const ElementBaseUrl: String);
begin
  if not WarmupCacheFormats.CallRegisteredFormat(Self, Element, ElementBaseUrl) then
    raise EInvalidSettingsXml.CreateFmt('Not recognized warmup cache element "%s"',
      [Element.TagName8]);
end;

{ globals -------------------------------------------------------------------- }

var
  FWarmupCacheFormats: TWarmupCacheFormatList;

function WarmupCacheFormats: TWarmupCacheFormatList;
begin
  if FWarmupCacheFormats = nil then
  begin
    FWarmupCacheFormats := TWarmupCacheFormatList.Create(true);
    // register formats implemented in this unit
    FWarmupCacheFormats.RegisterFormat('image_ui', @TImageUiCache(nil).Event);
  end;
  Result := FWarmupCacheFormats;
end;

procedure SettingsLoad(const Container: TUIContainer; const SettingsUrl: String);

  function UIScalingToString(const UIScaling: TUIScaling): String;
  begin
    Result := SEnding(GetEnumName(TypeInfo(TUIScaling), Ord(UIScaling)), 3);
  end;

  function UIScalingFromString(const S: String): TUIScaling;
  begin
    for Result := Low(TUIScaling) to High(TUIScaling) do
      if S = UIScalingToString(Result) then
        Exit;
    raise EInvalidSettingsXml.CreateFmt('Not a valid value for UIScaling: %s', [S]);
  end;

type
  TDynIntegerArray = array of Integer;

  function ParseIntegerList(const S: String): TDynIntegerArray;
  var
    IntegerList: TIntegerList;
    SeekPos: Integer;
    Token: String;
  begin
    IntegerList := TIntegerList.Create;
    try
      SeekPos := 1;
      repeat
        Token := NextToken(S, SeekPos);
        if Token = '' then Break;
        IntegerList.Add(StrToInt(Token));
      until false;

      if IntegerList.Count = 0 then
        raise EInvalidSettingsXml.Create('sizes_at_load parameter is an empty list in CastleSettings.xml');

      Result := IntegerList.ToArray;
    finally FreeAndNil(IntegerList) end;
  end;

  procedure ReadWarmupCache(E: TDOMElement);
  var
    Cache: TWarmupCache;
    I: TXMLElementIterator;
  begin
    Cache := TWarmupCache.Create(Container);
    I := E.ChildrenIterator;
    try
      while I.GetNext do
        Cache.ReadElement(I.Current, SettingsUrl);
    finally FreeAndNil(I) end;
  end;

const
  DefaultUIScaling = usNone;
  DefaultUIReferenceWidth = 0;
  DefaultUIReferenceHeight = 0;
var
  SettingsDoc: TXMLDocument;
  E: TDOMElement;

  // font stuff
  DefaultFontUrl: String;
  DefaultFontSize, DefaultFontLoadSize: Cardinal;
  DefaultFontAntiAliased: Boolean;
  NewDefaultFont: TCastleFont;
  AllSizesAtLoadStr: String;
  AllSizesAtLoad: TDynIntegerArray;

  NewUIScaling: TUIScaling;
  NewUIReferenceWidth, NewUIReferenceHeight: Single;
begin
  // initialize defaults
  NewUIScaling := DefaultUIScaling;
  NewUIReferenceWidth := DefaultUIReferenceWidth;
  NewUIReferenceHeight := DefaultUIReferenceHeight;
  NewDefaultFont := nil;

  SettingsDoc := URLReadXML(SettingsUrl);
  try
    if SettingsDoc.DocumentElement.TagName8 <> 'castle_settings' then
      raise EInvalidSettingsXml.Create('The root element must be <castle_settings>');

    E := SettingsDoc.DocumentElement.Child('ui_scaling', false);
    if E <> nil then
    begin
      NewUIScaling := UIScalingFromString(
        E.AttributeStringDef('mode', UIScalingToString(DefaultUIScaling)));
      NewUIReferenceWidth :=
        E.AttributeSingleDef('reference_width', DefaultUIReferenceWidth);
      NewUIReferenceHeight :=
        E.AttributeSingleDef('reference_height', DefaultUIReferenceHeight);
    end;

    E := SettingsDoc.DocumentElement.Child('default_font', false);
    if E <> nil then
    begin
      DefaultFontUrl := E.AttributeURL('url', SettingsUrl);
      DefaultFontSize := E.AttributeCardinalDef('size', 20);
      DefaultFontAntiAliased := E.AttributeBooleanDef('anti_aliased', true);

      if E.AttributeString('sizes_at_load', AllSizesAtLoadStr) then
      begin
        AllSizesAtLoad := ParseIntegerList(AllSizesAtLoadStr);
        NewDefaultFont := TCustomizedFont.Create(Container);
        TCustomizedFont(NewDefaultFont).Load(DefaultFontUrl, AllSizesAtLoad, DefaultFontAntiAliased);
      end else
      begin
        DefaultFontLoadSize := E.AttributeCardinalDef('size_at_load', DefaultFontSize);
        NewDefaultFont := TTextureFont.Create(Container);
        TTextureFont(NewDefaultFont).Load(DefaultFontUrl, DefaultFontLoadSize, DefaultFontAntiAliased);
      end;
      NewDefaultFont.Size := DefaultFontSize;
    end;

    E := SettingsDoc.DocumentElement.Child('warmup_cache', false);
    if E <> nil then
      ReadWarmupCache(E);
  finally FreeAndNil(SettingsDoc) end;

  Container.UIScaling := NewUIScaling;
  Container.UIReferenceWidth := NewUIReferenceWidth;
  Container.UIReferenceHeight := NewUIReferenceHeight;
  Container.DefaultFont := NewDefaultFont;
end;

finalization
  FreeAndNil(FWarmupCacheFormats);
end.
