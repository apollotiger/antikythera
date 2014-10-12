WIDTH = 288
HEIGHT = 336
svg = d3.select('#canvas').append('svg:svg').attr('width', WIDTH).attr('height', HEIGHT)

drawAstronomical = () ->
    d3.selectAll('svg > *').remove()
    background = svg.append('rect')
                    .attr('dx', '0')
                    .attr('dy', '0')
                    .attr('width', WIDTH)
                    .attr('height', HEIGHT)
                    .attr('fill', 'white')
                    .attr('stroke', '#888')
                    .attr('stroke-width', '2px')

    centerPosition = {x: WIDTH / 2, y: HEIGHT / 2}

    # draw dawn and dusk lines
    venusRadius = WIDTH * 0.3
    marsRadius = WIDTH * 0.35
    jupiterRadius = WIDTH * 0.4
    venusSize = 20
    marsSize = 20
    jupiterSize = 20
    sunWidth = 18
    astroGroup = svg.append('g').attr('class', 'astronomical')
    $.ajax({url: '/horizon', data: {lat: '37:52:18', lon: '-122:16:18'}, cached: false})
     .done((data) ->
         sunrise = data.rise_angle - (7 / 12 * Math.PI)
         sunset = data.set_angle - (7 / 12 * Math.PI)
         astroGroup.append('path')
                   .attr('id', 'horizon')
                   .attr('d', "M#{centerPosition.x + HEIGHT * -Math.sin(sunrise)},#{centerPosition.y + HEIGHT * -1 * Math.cos(sunrise)} L#{centerPosition.x},#{centerPosition.y} L#{centerPosition.x + HEIGHT * -Math.sin(sunset)},#{centerPosition.y + HEIGHT * -Math.cos(sunset)} L#{WIDTH},#{HEIGHT} L0,#{HEIGHT}")
                   .attr('stroke', 'black')
                   .attr('stroke-width', '4px')
    )



    $.ajax({url: '/angle', data: {body: 'sun', lat: '37:52:18', lon: '-122:16:18'}, cached: false})
     .done((data) ->
         sunPosition = data.angle
         astroGroup.append('path')
                   .attr('id', 'sunbeam')
                   .attr('d', "M#{centerPosition.x},#{centerPosition.y} L#{centerPosition.x + HEIGHT * -Math.cos(sunPosition - (sunWidth / 2) * Math.PI / 360.0)},#{centerPosition.y + HEIGHT * -Math.sin(sunPosition - (sunWidth / 2) * Math.PI / 360.0)} L#{centerPosition.x + HEIGHT * -Math.cos(sunPosition + (sunWidth / 2) * Math.PI / 360.0)},#{centerPosition.y + HEIGHT * -Math.sin(sunPosition + (sunWidth / 2) * Math.PI / 360.0)} L#{centerPosition.x},#{centerPosition.y} Z")
                   .attr('stroke', 'black')
                   .attr('stroke-width', '2px')
                   .attr('fill', 'white')
    )

    $.ajax({url: '/angle', data: {body: 'moon', lat: '37:52:18', lon: '-122:16:18'}, cached: false})
     .done((data) ->
         moonPosition = data.angle
         astroGroup.append('path')
                   .attr('id', 'moonbeam')
                   .attr('d', "M#{centerPosition.x},#{centerPosition.y} L#{centerPosition.x + HEIGHT * -Math.cos(moonPosition - (sunWidth / 2) * Math.PI / 360.0)},#{centerPosition.y + HEIGHT * -Math.sin(moonPosition - (sunWidth / 2) * Math.PI / 360.0)} L#{centerPosition.x + HEIGHT * -Math.cos(moonPosition + (sunWidth / 2) * Math.PI / 360.0)},#{centerPosition.y + HEIGHT * -Math.sin(moonPosition + (sunWidth / 2) * Math.PI / 360.0)} L#{centerPosition.x},#{centerPosition.y} Z")
                   .attr('stroke', 'white')
                   .attr('stroke-width', '2px')
                   .attr('fill', 'black')
    )

    $.ajax({url: '/angle', data: {body: 'venus', lat: '37:52:18', lon: '-122:16:18'}, cached: false})
     .done((data) ->
         venusPosition = data.angle
         astroGroup.append('path')
                   .attr('id', 'venus')
                   .attr('d', "M#{centerPosition.x + venusRadius * -Math.cos(venusPosition) - venusSize/2},#{centerPosition.y + venusRadius * -Math.sin(venusPosition) - venusSize/2} l#{venusSize},0 l-#{venusSize/2},#{venusSize} z")
                   .attr('stroke', 'black')
                   .attr('stroke-width', '2px')
                   .attr('fill', 'white')
    )

    $.ajax({url: '/angle', data: {body: 'mars', lat: '37:52:18', lon: '-122:16:18'}, cached: false})
     .done((data) ->
         marsPosition = data.angle
         astroGroup.append('path')
                   .attr('id', 'mars')
                   .attr('d', "M#{centerPosition.x + marsRadius * -Math.cos(marsPosition) - marsSize/2},#{centerPosition.y + marsRadius * -Math.sin(marsPosition) + marsSize/2} l#{marsSize},0 l-#{marsSize/2},-#{marsSize} z")
                   .attr('stroke', 'black')
                   .attr('stroke-width', '2px')
                   .attr('fill', 'white')
    )


    $.ajax({url: '/angle', data: {body: 'jupiter', lat: '37:52:18', lon: '-122:16:18'}, cached: false})
     .done((data) ->
         jupiterPosition = data.angle
         astroGroup.append('rect')
                   .attr('id', 'jupiter')
                   .attr('x', centerPosition.x + jupiterRadius * -Math.cos(jupiterPosition))
                   .attr('y', centerPosition.y + jupiterRadius * -Math.sin(jupiterPosition))
                   .attr('width', jupiterSize)
                   .attr('height', jupiterSize)
                   .attr('stroke', 'black')
                   .attr('stroke-width', '2px')
                   .attr('fill', 'white')
    )
    drawClock()


drawClock = () ->
    # draw the clock in the middle
    centerPosition = {x: WIDTH / 2, y: HEIGHT / 2}
    d3.selectAll('g.time > *').remove()
    timeGroup = svg.append('g').attr('class', 'time')
    currentTime = new Date
    hourAngle = (currentTime.getHours() % 12) / 6 * Math.PI
    if hourAngle == 0 then hourAngle = 2 * Math.PI - 0.000001
    if hourAngle < Math.PI then hourSweep = 0 else hourSweep = 1
    hourRadius = 10
    minuteAngle = (currentTime.getMinutes()) / 30 * Math.PI
    if minuteAngle == 0 then minuteAngle = 2 * Math.PI - 0.00001
    if minuteAngle < Math.PI then minuteSweep = 0 else minuteSweep = 1
    minuteRadius = 20
    secondMarkers = Math.floor(currentTime.getSeconds() / 2)
    secondRadius = 30

    timeGroup.append('circle')
             .attr('id', 'background-clock')
             .attr('cx', centerPosition.x)
             .attr('cy', centerPosition.y)
             .attr('r', secondRadius + 5)
             .attr('stroke', 'none')
             .attr('fill', 'white')
    timeGroup.append('path')
             .attr('id', 'hour-arc')
             .attr('d', "M#{centerPosition.x},#{centerPosition.y - hourRadius} A#{hourRadius},#{hourRadius} 0 #{hourSweep},1 #{centerPosition.x + hourRadius * Math.sin(hourAngle)},#{centerPosition.y - hourRadius * Math.cos(hourAngle)}")
             .attr('stroke', 'black')
             .attr('stroke-width', '4px')
             .attr('fill', 'none')
    timeGroup.append('path')
             .attr('id', 'minute-arc')
             .attr('d', "M#{centerPosition.x},#{centerPosition.y - minuteRadius} A#{minuteRadius},#{minuteRadius} 0 #{minuteSweep},1 #{centerPosition.x + minuteRadius * Math.sin(minuteAngle)},#{centerPosition.y - minuteRadius * Math.cos(minuteAngle)}")
             .attr('stroke', 'black')
             .attr('stroke-width', '4px')
             .attr('fill', 'none')

    secondGroup = timeGroup.append('g')
                           .attr('class', 'second-group')

    if secondMarkers % 30 != 0
        if secondMarkers <= 15
            for secondMarker in [1..secondMarkers]
                do (secondMarker) ->
                    blip = secondMarker * 4
                    secondGroup.append('circle')
                               .attr('cx', centerPosition.x + secondRadius * Math.sin(blip / 30 * Math.PI))
                               .attr('cy', centerPosition.y - secondRadius * Math.cos(blip / 30 * Math.PI))
                               .attr('fill', 'black')
                               .attr('stroke', 'none')
                               .attr('r', '4px')
        else
            for secondMarker in [(secondMarkers % 15 + 1)..15]
                do (secondMarker) ->
                    blip = secondMarker * 4
                    secondGroup.append('circle')
                               .attr('cx', centerPosition.x + secondRadius * Math.sin(blip / 30 * Math.PI))
                               .attr('cy', centerPosition.y - secondRadius * Math.cos(blip / 30 * Math.PI))
                               .attr('fill', 'black')
                               .attr('stroke', 'none')
                               .attr('r', '4px')

drawAstronomical()
setInterval(drawClock, 500)
setInterval(drawAstronomical, 300000)
