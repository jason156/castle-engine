{%MainUnit castlepropedits.pas}
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

{ Vector property editors. }

{ TCastleVector3PropertyEditor ------------------------------------------------- }

type
  TCastleVector3PropertyEditor = class(TSubPropertiesEditor)
    function GetAttributes: TPropertyAttributes; override;
    function GetValue: String; override;
    procedure SetValue(const NewValue: String); override;
  end;

function TCastleVector3PropertyEditor.GetAttributes: TPropertyAttributes;
begin
  Result := inherited GetAttributes + [paRevertable]
    // our SetValue allows to set value from string
    - [paReadOnly];
end;

function TCastleVector3PropertyEditor.GetValue: String;
var
  VectorPersistent: TCastleVector3Persistent;
begin
  VectorPersistent := (GetObjectValue as TCastleVector3Persistent);
  Result := VectorPersistent.Value.ToString;
end;

procedure TCastleVector3PropertyEditor.SetValue(const NewValue: String);
var
  VectorPersistent: TCastleVector3Persistent;
begin
  VectorPersistent := (GetObjectValue as TCastleVector3Persistent);
  VectorPersistent.Value := Vector3FromStr(NewValue);
end;

{ TCastleVector4PropertyEditor ------------------------------------------------- }

type
  TCastleVector4PropertyEditor = class(TSubPropertiesEditor)
    function GetAttributes: TPropertyAttributes; override;
    function GetValue: String; override;
    procedure SetValue(const NewValue: String); override;
  end;

function TCastleVector4PropertyEditor.GetAttributes: TPropertyAttributes;
begin
  Result := inherited GetAttributes + [paRevertable]
    // our SetValue allows to set value from string
    - [paReadOnly];
end;

function TCastleVector4PropertyEditor.GetValue: String;
var
  VectorPersistent: TCastleVector4Persistent;
begin
  VectorPersistent := (GetObjectValue as TCastleVector4Persistent);
  Result := VectorPersistent.Value.ToString;
end;

procedure TCastleVector4PropertyEditor.SetValue(const NewValue: String);
var
  VectorPersistent: TCastleVector4Persistent;
begin
  VectorPersistent := (GetObjectValue as TCastleVector4Persistent);
  VectorPersistent.Value := Vector4FromStr(NewValue);
end;
