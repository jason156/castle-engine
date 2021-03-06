{%MainUnit castlecontrols.pas}
{
  Copyright 2018-2018 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{$ifdef read_interface}

  { Checkbox with a caption. }
  TCastleCheckbox = class(TCastleUserInterface)
  strict private
    FChecked, FPressed: Boolean;
    FCaption: String;
    FOnChange: TNotifyEvent;
    FCaptionMargin: Single;
    FImageContainer: TCastleUserInterface;
    FImageSquareEmpty, FImageSquareChecked, FImagePressedBackground: TCastleImageControl;
    FLabelCaption: TCastleLabel;
    FTextColor, FCheckboxColor: TCastleColor;
    FAutoSize: Boolean;
    FFontSize: Single;
    FCheckboxSize: Single;
    procedure SetChecked(const Value: Boolean);
    procedure SetCaption(const Value: String);
    procedure SetTextColor(const Value: TCastleColor);
    procedure SetCheckboxColor(const Value: TCastleColor);
    procedure SetCaptionMargin(const Value: Single);
    procedure SetAutoSize(const Value: Boolean);
    procedure SetFontSize(const Value: Single);
    procedure SetCheckboxSize(const Value: Single);
  protected
    procedure PreferredSize(var PreferredWidth, PreferredHeight: Single); override;
    function GetInternalText: String; override;
    procedure SetInternalText(const Value: String); override;
  public
    const
      DefaultCaptionMargin = 10;
      DefaultCheckboxSize = 24;
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    function Press(const Event: TInputPressRelease): boolean; override;
    function Release(const Event: TInputPressRelease): boolean; override;
    function PropertySection(const PropertyName: String): TPropertySection; override;
    procedure EditorAllowResize(
      out ResizeWidth, ResizeHeight: Boolean; out Reason: String); override;

    { Color of the label. By default opaque black. }
    property TextColor: TCastleColor read FTextColor write SetTextColor;
    { Color of the checkbox square and checkmark. By default opaque black. }
    property CheckboxColor: TCastleColor read FCheckboxColor write SetCheckboxColor;
  published
    property Checked: Boolean read FChecked write SetChecked default false;
    property Caption: String read FCaption write SetCaption;
    { Margin between checkbox square and a text @link(Caption). }
    property CaptionMargin: Single read FCaptionMargin write SetCaptionMargin
      default DefaultCaptionMargin;
    { Event sent when @link(Checked) value was changed by a user click.
      Note that this is @italic(not) called when you change @link(Checked)
      property programmatically. }
    property OnChange: TNotifyEvent read FOnChange write FOnChange;
    { Should our size be determined by checkbox image size and caption. }
    property AutoSize: Boolean read FAutoSize write SetAutoSize default true;
    { Caption font size. }
    property FontSize: Single read FFontSize write SetFontSize default 0.0;
    { Size of the square checkbox. }
    property CheckboxSize: Single read FCheckboxSize write SetCheckboxSize
      default DefaultCheckboxSize;

  {$define read_interface_class}
  {$I auto_generated_persistent_vectors/tcastlecheckbox_persistent_vectors.inc}
  {$undef read_interface_class}
  end;

{$endif read_interface}

{$ifdef read_implementation}

{ TCastleCheckbox ---------------------------------------------------------- }

constructor TCastleCheckbox.Create(AOwner: TComponent);
begin
  inherited;
  FCaptionMargin := DefaultCaptionMargin;
  FAutoSize := true;
  FTextColor := Black;
  FCheckboxColor := Black;
  FCheckboxSize := DefaultCheckboxSize;

  FImageContainer := TCastleUserInterface.Create(Self);
  FImageContainer.SetTransient;
  FImageContainer.Width := FCheckboxSize;
  FImageContainer.Height := FCheckboxSize;
  FImageContainer.Anchor(vpMiddle);
  InsertFront(FImageContainer);

  { TODO: Use
    Theme.Corners[tiSquarePressedBackground]
    Theme.Corners[tiSquareEmpty]
    Theme.Corners[tiSquareChecked]
    Make sure they cost nothing when corners are 0. }

  FImagePressedBackground := TCastleImageControl.Create(Self);
  FImagePressedBackground.SetTransient;
  FImagePressedBackground.Image := Theme.Images[tiSquarePressedBackground];
  FImagePressedBackground.OwnsImage := false;
  FImagePressedBackground.Color := FCheckboxColor;
  FImagePressedBackground.Exists := FPressed;
  FImagePressedBackground.Stretch := true;
  FImagePressedBackground.FullSize := true; // size is determined by FImageContainer
  FImageContainer.InsertFront(FImagePressedBackground);

  FImageSquareEmpty := TCastleImageControl.Create(Self);
  FImageSquareEmpty.SetTransient;
  FImageSquareEmpty.Image := Theme.Images[tiSquareEmpty];
  FImageSquareEmpty.OwnsImage := false;
  FImageSquareEmpty.Color := FCheckboxColor;
  FImageSquareEmpty.Exists := not FChecked;
  FImageSquareEmpty.Stretch := true;
  FImageSquareEmpty.FullSize := true; // size is determined by FImageContainer
  FImageContainer.InsertFront(FImageSquareEmpty);

  FImageSquareChecked := TCastleImageControl.Create(Self);
  FImageSquareChecked.SetTransient;
  FImageSquareChecked.Image := Theme.Images[tiSquareChecked];
  FImageSquareChecked.OwnsImage := false;
  FImageSquareChecked.Color := FCheckboxColor;
  FImageSquareChecked.Exists := FChecked;
  FImageSquareChecked.Stretch := true;
  FImageSquareChecked.FullSize := true; // size is determined by FImageContainer
  FImageContainer.InsertFront(FImageSquareChecked);

  FLabelCaption := TCastleLabel.Create(Self);
  FLabelCaption.SetTransient;
  FLabelCaption.Color := FTextColor;
  FLabelCaption.Anchor(hpLeft, FImageContainer.EffectiveWidth + CaptionMargin);
  FLabelCaption.Anchor(vpMiddle);
  InsertFront(FLabelCaption);

  {$define read_implementation_constructor}
  {$I auto_generated_persistent_vectors/tcastlecheckbox_persistent_vectors.inc}
  {$undef read_implementation_constructor}
end;

destructor TCastleCheckbox.Destroy;
begin
  {$define read_implementation_destructor}
  {$I auto_generated_persistent_vectors/tcastlecheckbox_persistent_vectors.inc}
  {$undef read_implementation_destructor}
  inherited;
end;

procedure TCastleCheckbox.PreferredSize(var PreferredWidth, PreferredHeight: Single);
begin
  if AutoSize then
  begin
    PreferredWidth := UIScale * (
      FImageContainer.EffectiveWidth + CaptionMargin +
      FLabelCaption.EffectiveWidth);
    PreferredHeight := UIScale *
      Max(FImageContainer.EffectiveHeight, FLabelCaption.EffectiveHeight);
  end;
end;

procedure TCastleCheckbox.EditorAllowResize(
  out ResizeWidth, ResizeHeight: Boolean; out Reason: String);
begin
  inherited;
  if AutoSize then
  begin
    ResizeWidth := false;
    ResizeHeight := false;
    Reason := SAppendPart(Reason, NL, 'Turn off "TCastleCheckbox.AutoSize" to change size.');
  end;
end;

procedure TCastleCheckbox.SetChecked(const Value: boolean);
begin
  if FChecked <> Value then
  begin
    FChecked := Value;
    FImageSquareEmpty.Exists := not FChecked;
    FImageSquareChecked.Exists := FChecked;
    { Note that we don't call "VisibleChange([chRender])" here,
      because the change to "FImageSquareChecked.Exists" should already call it.
      In general, since TCastleCheckbox doesn't override Render method,
      it doesn't need to worry about calling "VisibleChange([chRender])". }
  end;
end;

function TCastleCheckbox.Press(const Event: TInputPressRelease): boolean;
begin
  Result := inherited;
  if Result then Exit;

  if Event.IsMouseButton(mbLeft) or
     Event.IsKey(keySpace) or
     Event.IsKey(keyEnter) then
  begin
    FPressed := true;
    FImagePressedBackground.Exists := FPressed;
    Exit(ExclusiveEvents);
  end;
end;

function TCastleCheckbox.Release(const Event: TInputPressRelease): boolean;
begin
  Result := inherited;
  if Result then Exit;

  if FPressed and (
     Event.IsMouseButton(mbLeft) or
     Event.IsKey(keySpace) or
     Event.IsKey(keyEnter)) then
  begin
    FPressed := false;
    FImagePressedBackground.Exists := FPressed;

    { Because of mouse capture mechanism in TUIContainer,
      we receive Release notifications about all clicks that started
      within our rect, regardless where they finished.

      Just like TCastleButton, the click only counts
      if it happens over our rect. }
    if CapturesEventsAtPosition(Event.Position) then
    begin
      // change using property, so FImageSquareChecked/Empty.Exists is switched
      Checked := not Checked;
      if Assigned(FOnChange) then
        FOnChange(Self);
    end;

    Exit(ExclusiveEvents);
  end;
end;

procedure TCastleCheckbox.SetCaption(const Value: String);
begin
  if FCaption <> Value then
  begin
    FCaption := Value;
    FLabelCaption.Caption := Value;
  end;
end;

procedure TCastleCheckbox.SetTextColor(const Value: TCastleColor);
begin
  if not TCastleColor.PerfectlyEquals(FTextColor, Value) then
  begin
    FTextColor := Value;
    FLabelCaption.Color := Value;
  end;
end;

procedure TCastleCheckbox.SetCheckboxColor(const Value: TCastleColor);
begin
  if not TCastleColor.PerfectlyEquals(FCheckboxColor, Value) then
  begin
    FCheckboxColor := Value;
    FImageSquareEmpty.Color := Value;
    FImageSquareChecked.Color := Value;
    FImagePressedBackground.Color := Value;
  end;
end;

procedure TCastleCheckbox.SetCheckboxSize(const Value: Single);
begin
  if FCheckboxSize <> Value then
  begin
    FCheckboxSize := Value;
    FImageContainer.Width := Value;
    FImageContainer.Height := Value;
    FLabelCaption.Anchor(hpLeft, FImageContainer.EffectiveWidth + CaptionMargin);
  end;
end;

procedure TCastleCheckbox.SetCaptionMargin(const Value: Single);
begin
  if FCaptionMargin <> Value then
  begin
    FCaptionMargin := Value;
    FLabelCaption.Anchor(hpLeft, FImageContainer.EffectiveWidth + CaptionMargin);
  end;
end;

procedure TCastleCheckbox.SetFontSize(const Value: Single);
begin
  if FFontSize <> Value then
  begin
    FFontSize := Value;
    FLabelCaption.FontSize := Value;
  end;
end;

procedure TCastleCheckbox.SetAutoSize(const Value: Boolean);
begin
  if FAutoSize <> Value then
  begin
    FAutoSize := Value;
    VisibleChange([chRectangle]); // our effective size changed now
  end;
end;

function TCastleCheckbox.GetInternalText: String;
begin
  Result := Caption;
end;

procedure TCastleCheckbox.SetInternalText(const Value: String);
begin
  Caption := Value;
end;

function TCastleCheckbox.PropertySection(
  const PropertyName: String): TPropertySection;
begin
  case PropertyName of
    'Caption', 'Checked':
      Result := psBasic;
    'AutoSize':
      Result := psLayout;
    else
      Result := inherited PropertySection(PropertyName);
  end;
end;

{$define read_implementation_methods}
{$I auto_generated_persistent_vectors/tcastlecheckbox_persistent_vectors.inc}
{$undef read_implementation_methods}

{$endif read_implementation}
